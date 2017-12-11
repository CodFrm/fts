#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string>
#include <time.h>
#include "mysql.h"
std::string randStr(int len);
class fts{
public:
    static void* acceptThread(void*);

    static void* dealThread(void*);
    int init(int port);

    static bool createFileCache(const char*,int64_t);
private:
    int m_sSocket;
    pthread_t  m_phAccept;

protected:

};