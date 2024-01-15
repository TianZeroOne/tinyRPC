#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/comm/log.h"

int main() {
    tinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyRPC::Logger::InitGlobalLogger();

    tinyRPC::IPNetAddr addr("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr.toString().c_str());
    
}