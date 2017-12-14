#include "mysql.h"
//debug
#define _DEBUG
#ifdef _DEBUG
#include <stdio.h>
#define prt(s,p) printf(s,p)
#else
#define prt(s,p)
#endif


mysql::mysql(){
    m_con=mysql_init(NULL);
    if(!mysql_real_connect(m_con,db_host,db_user,db_pwd,db_db,3306,NULL,0)){
        prt("error db connect\n",m_con);
    }
    exec("set names utf8;");
    prt("db connect..\n",m_con);
}

mysql::~mysql(){
    printf("db close\n");
    mysql_close(m_con);
}
bool mysql::exec(const char* sql){
    return exec(sql,NULL);
}
bool mysql::exec(const char* sql,MYSQL_BIND* param){
    MYSQL_STMT *stmt;
    stmt=mysql_stmt_init(m_con);
    if(mysql_stmt_prepare(stmt,sql,strlen(sql))){
        mysql_stmt_close(stmt);
        return false;
    }
    if(mysql_stmt_bind_param(stmt,param)){
        mysql_stmt_close(stmt);
        return false;
    }
    if(mysql_stmt_execute(stmt)){
        mysql_stmt_close(stmt);
        return false;
    }
    mysql_stmt_close(stmt);
    return true;
}

record* mysql::query(const char* sql){
    return query(sql,NULL);
}
record* mysql::query(const char* sql,MYSQL_BIND* param){
    MYSQL_STMT *stmt;
    stmt=mysql_stmt_init(m_con);
    record* tmpRec=new record(stmt);
    if(mysql_stmt_prepare(stmt,sql,strlen(sql))){
        mysql_stmt_close(stmt);
        return NULL;
    }
    if(mysql_stmt_bind_param(stmt,param)){
        mysql_stmt_close(stmt);
        return NULL;
    }
    if(mysql_stmt_execute(stmt)){
        mysql_stmt_close(stmt);
        return NULL;
    }

    return tmpRec;
}

const char* mysql::error(){
    return mysql_error(m_con);
}

record::record(MYSQL_STMT* stmt){
    m_pSTMT=stmt;
    m_Bind=NULL;
}

record::~record(){
    mysql_stmt_close(m_pSTMT);
    if(m_Bind){
        for(int i=0;i<count;i++){
            free(m_Bind[i].buffer);
        }
        free(m_Bind);
    }
}

void** record::fetch(){
    if(!m_Bind){
        MYSQL_RES* meta_result=mysql_stmt_result_metadata(m_pSTMT);
        unsigned long len=0;
        if(meta_result){
            count=mysql_num_fields(meta_result);
            m_Bind=(MYSQL_BIND*)malloc(sizeof(MYSQL_BIND)*count);
            memset(m_Bind,0,sizeof(MYSQL_BIND)*count);
            m_Result=new void*[count];
            MYSQL_FIELD* field;
            int pos=0;
            while(field=mysql_fetch_field(meta_result)){
                m_Bind[pos].length=&len;
                if(field->type==FIELD_TYPE_LONG){
                    m_Result[pos]=malloc(sizeof(int));
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_LONG;
                }else if(field->type==253){
                    m_Result[pos]=malloc(255);
                    memset(m_Result[pos],0,255);
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_STRING;
                    m_Bind[pos].buffer_length=255;
                    len=255;
                    m_Bind[pos].length=&len;
                }else if(field->type==FIELD_TYPE_DOUBLE){
                    m_Result[pos]=malloc(sizeof(double));
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_DOUBLE;
                }else if(field->type==FIELD_TYPE_LONGLONG){
                    m_Result[pos]=malloc(sizeof(int64_t));
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_LONGLONG;
                }
                pos++;
            }
            mysql_free_result(meta_result);
        }
    }
    if(mysql_stmt_bind_result(m_pSTMT,m_Bind)){
        return NULL;
    }
    printf("fetch\n");
    try{
        int r=0;
        if(r=mysql_stmt_fetch(m_pSTMT)){
            printf("r:%d,%d,%d\n",r,MYSQL_NO_DATA,MYSQL_DATA_TRUNCATED);
            return NULL;
        }
    }catch(...){
        printf("fetch error\n");
        return NULL;
    }
    return m_Result;
}