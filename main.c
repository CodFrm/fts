#include <stdio.h>
#include "fts.h"

int main(){
    printf("------file transfer server start-----\n");
    fts* server=new fts();
    server->init(5209);
    return 0;
}
