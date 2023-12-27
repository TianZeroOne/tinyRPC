#include <pthread.h>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/config.h"

void* fun(void*) {
    int i = 20;
    while (i--) {
        DEBUGLOG("tis new thread debug log %s", "fun");
        INFOLOG("%d this new thread info info log %s", i, "fun");
    }
    

    return NULL;
}
int main() {

    tinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyRPC::Logger::InitGlobalLogger();
    pthread_t thread;
    pthread_create(&thread, NULL, &fun, NULL);
    
    int i = 20;
    while (i--) {
        DEBUGLOG("test debug log %s", "11");
        INFOLOG("%d test info log %s", i, "11");
    }
    
    pthread_join(thread, NULL);
    return 0;
}