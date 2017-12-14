#include <mysql/mysql.h>
#include <string.h>
#include <malloc.h>

#define db_host "10.127.133.247"
#define db_user "root"
#define db_pwd ""
#define db_db "jx"

class record;

class mysql{
private:
    MYSQL* m_con;
public:
    mysql();
    ~mysql();

    record* query(const char* sql);
    record* query(const char* sql,MYSQL_BIND*);

    bool exec(const char* sql);
    bool exec(const char* sql,MYSQL_BIND*);

    const char* error();
};

class record{
private:
    MYSQL_STMT* m_pSTMT=NULL;
    MYSQL_BIND* m_Bind=NULL;
    void** m_Result=NULL;
    int count=0;
public:
    record(MYSQL_STMT*);
    ~record();

    void** fetch();
};