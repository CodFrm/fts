#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include "mysql.h"

class fts{
public:
    static void* acceptThread(void*);

    static void* dealThread(void*);
    int init(int port);


private:
    int m_sSocket;
    pthread_t  m_phAccept;

protected:

};