#include "fts.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

//debug
#define _DEBUG
#ifdef _DEBUG
#include <stdio.h>
#define prt(s) printf(s)
#else
#define prt(s)
#endif


int fts::init(int port)
{
    int server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(port);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ///bind，成功返回0，出错返回-1
    if (bind(server_sockfd, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1)
    {
        return -1;
    }

    ///listen，成功返回0，出错返回-1
    if (listen(server_sockfd, 5) == -1)
    {
        return -1;
    }
    pthread_t id;
    int ret=pthread_create(&id,NULL,fts::acceptThread,(void*)server_sockfd);
    if(ret!=0)return ret;
    pthread_join(id,NULL);
    return 0;
}

void* fts::acceptThread(void* p){
    int* sServer=(int*)p;
    prt("--debug--\n");
    int sClient;
    while((sClient = accept(*sServer,(struct sockaddr*)NULL,NULL))==-1){
        
    }
    return 0;
}