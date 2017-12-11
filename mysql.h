#include <mysql/mysql.h>
#include <string.h>

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

    record* query(char* sql);
    record* query(char* sql,MYSQL_BIND*);

    const char* error();
};

class record{
private:
    MYSQL_STMT* m_pSTMT=NULL;
    MYSQL_BIND* m_Bind=NULL;
    void** m_Result=NULL;
public:
    record(MYSQL_STMT*);
    ~record();

    void** fetch();
};