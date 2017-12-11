#include <stdio.h>
#include "fts.h"
// #include "mysql.h"

int main(){
    // use mysql demo
    // mysql* tmp=new mysql();
    // record* tmpRec;
    // if(tmpRec=tmp->query("select * from jx_user;")){
    //     printf("success\n",tmp);
    //     void** res=tmpRec->fetch();
    //     printf("uid:%d,user:%s,pwd:%s\n",*(int*)res[0],res[1],res[2]);
    // }else{
    //     printf("error %s\n",tmp->error());
    // }

    fts* server=new fts();
    printf("------file transfer server start-----\n");
    server->init(5209);
    return 0;
}
