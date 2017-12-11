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
    prt("db connect..\n",m_con);
}

mysql::~mysql(){
    mysql_close(m_con);
}
record* mysql::query(char* sql){
    return query(sql,NULL);
}
record* mysql::query(char* sql,MYSQL_BIND* param){
    MYSQL_STMT *stmt;
    stmt=mysql_stmt_init(m_con);
    record* tmpRec=new record(stmt);
    if(mysql_stmt_prepare(stmt,sql,strlen(sql))){
        return NULL;
    }
    if(mysql_stmt_bind_param(stmt,param)){
        return NULL;
    }
    if(mysql_stmt_execute(stmt)){
        return NULL;
    }
    return tmpRec;
}

const char* mysql::error(){
    return mysql_error(m_con);
}

record::record(MYSQL_STMT* stmt){
    m_pSTMT=stmt;
}

record::~record(){
    mysql_stmt_close(m_pSTMT);
    free(m_Bind);
}

void** record::fetch(){
    if(!m_Bind){
        MYSQL_RES* meta_result=mysql_stmt_result_metadata(m_pSTMT);
        unsigned long len;
        if(meta_result){
            int count=mysql_num_fields(meta_result);
            m_Bind=(MYSQL_BIND*)malloc(sizeof(MYSQL_BIND)*count);
            m_Result=new void*[count];
            MYSQL_FIELD* field;
            int pos=0;
            while(field=mysql_fetch_field(meta_result)){
                m_Bind[pos].length=&len;
                if(field->type==FIELD_TYPE_LONG){
                    m_Result[pos]=malloc(4);
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_LONG;
                }else if(field->type==253){
                    m_Result[pos]=malloc(field->length);
                    memset(m_Result[pos],0,field->length);
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_STRING;
                    m_Bind[pos].buffer_length=field->length;
                    len=field->length;
                    m_Bind[pos].length=&len;
                }else if(field->type==FIELD_TYPE_DOUBLE){
                    m_Result[pos]=malloc(8);
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_DOUBLE;
                }else if(field->type==FIELD_TYPE_LONGLONG){
                    m_Result[pos]=malloc(8);
                    m_Bind[pos].buffer=(char*)m_Result[pos];
                    m_Bind[pos].buffer_type=MYSQL_TYPE_LONGLONG;
                }
                pos++;
            }
        }
        mysql_free_result(meta_result);
    }
    if(mysql_stmt_bind_result(m_pSTMT,m_Bind)){
        return NULL;
    }
    if(mysql_stmt_fetch(m_pSTMT)){
        return NULL;
    }
    return m_Result;
}