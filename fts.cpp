#include "fts.h"

//debug
#define _DEBUG
#ifdef _DEBUG
#include <stdio.h>
#define prt(s,p) printf(s,p)
#else
#define prt(s,p)
#endif

#define dir "tmp/"

int fts::init(int port)
{
    this->m_sSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ///bind，成功返回0，出错返回-1
    if (bind(this->m_sSocket, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        return -1;
    }
    ///listen，成功返回0，出错返回-1
    if (listen(this->m_sSocket, 5) == -1)
    {
        return -1;
    }
    int ret=pthread_create(&this->m_phAccept,NULL,fts::acceptThread,(void*)&this->m_sSocket);
    if(ret!=0)return ret;
    pthread_join(this->m_phAccept,NULL);
    return 0;
}

void* fts::acceptThread(void* p){
    int sServer=*(int*)p;
    int sClient=0;
    while((sClient = accept(sServer,(struct sockaddr*)NULL,NULL))>0){
        prt("client connect:%d\n",sClient);
        pthread_t id;
        pthread_create(&id,NULL,fts::dealThread,(void*)&sClient);
    }
    return 0;
}

std::string randStr(int len){
    srand((unsigned)time(NULL));
    char str[]="zxcvbnmasdfghjklqwertyuiop1234567890QWERTYUIOPASDFGHJKLZXCVBNM";
    std::string ret;
    for(int i=0;i<len;i++){
        ret+=str[(rand()%(sizeof(str)-1))];
    }
    return ret;
}

void* fts::dealThread(void* p){
    int sClient=*(int*)p;
    char headRecvBuff[1024];
    if(recv(sClient,headRecvBuff,1024,0)<=0){
       return 0;
    }
    int64_t fileRecvLen=0;
    char code=headRecvBuff[0];
    int64_t fileSize=*(int64_t*)&headRecvBuff[1];
    int64_t soft_id=*(int64_t*)&headRecvBuff[9];
    int64_t max=4294967296;//max 4GB
    if(fileSize<=0 || fileSize>=max){
        char tmp[]=" file size is too large(0~4GB)";
        tmp[0]=1;
        send(sClient,tmp,sizeof(tmp),0);
        prt("file size error:%ld\n",fileSize);
        return 0;
    }
    prt("code:%d,",code);
    prt("soft_id:%d,",soft_id);
    prt("file size:%ld\n",fileSize);
    mysql db;
    record* tmpRec;
    MYSQL_BIND* param;
    param=(MYSQL_BIND*)malloc(sizeof(MYSQL_BIND)*1);
    unsigned long len=8;
    memset(param,0,sizeof(param));
    param[0].buffer_type=MYSQL_TYPE_LONGLONG;
    param[0].buffer=(char*)&soft_id;
    param[0].length=&len;
    FILE* pFile;
    std::string tmpPath=dir;
    printf("soft_id:%d    ------\n",soft_id);
    std::string sql="select `soft_uid`,`soft_filename`,`soft_type` from jx_soft_list where sid=";
    char tmp[32];
    sprintf(tmp,"%ld",soft_id);
    sql+=tmp;
    if(tmpRec=db.query(sql.c_str())){
        free(param);
        void** res=tmpRec->fetch();
        if(!res){
            printf("error:%s\n",db.error());
            delete tmpRec;
            char tmp[]=" could not find sid";
            tmp[0]=2;
            send(sClient,tmp,sizeof(tmp),0);
            prt("could not find sid:%ld\n",soft_id);
            return 0;
        }
        printf("uid:%ld,filename:%s\n",*(int64_t*)res[0],res[1]);
        if((*(int*)res[2])!=3){
            delete tmpRec;
            char tmp[]=" could not find sid";
            tmp[0]=2;
            send(sClient,tmp,sizeof(tmp),0);
            prt("could not find sid:%ld\n",soft_id);
            return 0;
        }
        //create file
        delete tmpRec;
        std::string filename="";
        if(code==1){
            filename=randStr(16)+".tmp";
            tmpPath+=filename;
            if(!fts::createFileCache(tmpPath.c_str(),fileSize)){
                char tmp[]=" server disk error";
                tmp[0]=4;
                send(sClient,tmp,sizeof(tmp),0);
                return 0;
            }
        }else if(code==2){
            //code=2 continue send
            fileRecvLen=*(int64_t*)&headRecvBuff[17];
            filename=&headRecvBuff[25];
            tmpPath+=filename;
        }
        printf("file:%s len:%ld\n",tmpPath.c_str(),fileRecvLen);
        if(!(pFile=fopen(tmpPath.c_str(),"rx+"))){
            char tmp[]=" server system error";
            tmp[0]=5;
            send(sClient,tmp,sizeof(tmp),0);
            return 0;
        }      
        if(fseek(pFile,fileRecvLen,0)){
            char tmp[]=" continue param error";
            tmp[0]=6;
            send(sClient,tmp,sizeof(tmp),0);
            return 0;
        }  
        std::string strSend=" ";
        strSend+=filename;
        char tmp[32]={0};
        strcpy(tmp,strSend.c_str());
        printf("send:%s\n",strSend.c_str());
        tmp[0]=10;
        send(sClient,tmp,strSend.length(),0);
    }else{
        printf("sql error");
        free(param);
        char tmp[]=" could not find sid";
        tmp[0]=2;
        send(sClient,tmp,sizeof(tmp),0);
        prt("could not find sid:%ld\n",soft_id);
        return 0;
    }
    int64_t recvTotalLen=fileRecvLen;
    int64_t recvLen=0;
    printf("start recv file temp file:%s\n",tmpPath.c_str());
    while(true){
        char recvBuff[4096]={0};//cache
        if((recvLen=recv(sClient,recvBuff,4096,0))<=0)break;
        recvTotalLen+=recvLen;
        fwrite(recvBuff,recvLen,1,pFile);
        if(recvTotalLen>=fileSize){
            char tmp[]=" success";
            tmp[0]=0;
            send(sClient,tmp,sizeof(tmp),0);
            printf("%ld,%ld\n",recvTotalLen,fileSize);
            fclose(pFile);
            //update db
            param=(MYSQL_BIND*)malloc(sizeof(MYSQL_BIND)*2);
            memset(param,0,sizeof(MYSQL_BIND)*2);
            char pchr[32]={0};
            unsigned long strLen=tmpPath.length();
            strcpy(pchr,tmpPath.c_str());
            param[0].buffer_type=MYSQL_TYPE_STRING;
            param[0].buffer=(char*)pchr;
            param[0].buffer_length=tmpPath.length();
            param[0].length=&strLen;
            printf("%s %d\n",pchr,tmpPath.length());
            param[1].buffer_type=MYSQL_TYPE_LONGLONG;
            param[1].buffer=(char*)&soft_id;
            param[1].length=0;
            if(!db.exec("update jx_soft_list set `soft_path`=?,`soft_type`=0 where sid=?",param)){
                printf("update error,path:%s sid:%ld\n",pchr,soft_id);
            }
            free(param);
            shutdown(sClient,2);
            break;
        }
    }
    printf("close connect:%d\n",sClient);
    return 0;
}


bool fts::createFileCache(const char* file,int64_t size){
    FILE* pFile;
    if(!(pFile=fopen(file,"wx"))){
        return false;
    }
    int64_t total=size;
    int w=4194304;
    char* buffer=new char[4194304];
    memset(buffer,0,4194304);
    while(total>0){
        total-=4194304;
        if(total<0){
            w=total+4194304;
        }
        if(fwrite(buffer,w,1,pFile)<=0){
            delete[] buffer;
            fclose(pFile);
            remove(file);
            return false;
        }
    }
    delete[] buffer;
    fclose(pFile);
    return true;
}