#include <memory>
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/comm/log.h"
#include "tinyRPC/net/tcp/tcp_server.h"

void test_tcp_server() {
    tinyRPC::IPNetAddr::s_ptr addr = std::make_shared<tinyRPC::IPNetAddr>("127.0.0.1", 12345);
    DEBUGLOG("create addr %s", addr->toString().c_str());

    tinyRPC::TcpServer tcp_server(addr);

    tcp_server.start();

}
int main() {
    tinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyRPC::Logger::InitGlobalLogger();
    test_tcp_server();
    
    
    
}