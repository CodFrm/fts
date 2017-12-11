#include "fts.h"

//debug
#define _DEBUG
#ifdef _DEBUG
#include <stdio.h>
#define prt(s,p) printf(s,p)
#else
#define prt(s,p)
#endif


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

void* fts::dealThread(void* p){
    int sClient=*(int*)p;
    char headRecvBuff[1024];
    if(recv(sClient,headRecvBuff,1024,0)<=0){
       return 0;
    }
    char code=headRecvBuff[0];
    int64_t fileSize=*(int64_t*)&headRecvBuff[1];
    int64_t max=4294967296;//max 4GB
    if(fileSize<=0 || fileSize>=max){
        char tmp[]=" file size is too large(0~4GB)";
        tmp[0]=1;
        send(sClient,tmp,sizeof(tmp),0);
        prt("file size error:%ld\n",fileSize);
        return 0;
    }
    std::string filename=&headRecvBuff[9];
    std::string token=&headRecvBuff[9+filename.length()+1];
    prt("code:%d,",code);
    prt("filename:%s,",filename.c_str());
    prt("token:%s,",token.c_str());
    prt("file size:%ld\n",fileSize);
    while(true){
        char recvBuff[1024];
        if(recv(sClient,recvBuff,1024,0)<=0)break;
        prt("data:%s\n",recvBuff);
    }
    prt("close connect:%d\n",sClient);
    return 0;
}
