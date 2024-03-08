#include <assert.h>
#include <string.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/config.h"
#include "tinyRPC/net/tcp/tcp_connection.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/string_coder.h"
#include "tinyRPC/net/tcp/abstract_protocol.h"

void test_connect() {
    // 调用 connect 连接 server
    // write 一个字符串
    // 等待 read 返回结果

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        ERRORLOG("invalid fd %d", fd);
        exit(0);
    }

    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(12345);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int rt = connect(fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));

    std::string msg = "hello tinyRPC!";

    rt = write(fd, msg.c_str(), msg.length());
    DEBUGLOG("success write %d bytes, [%s]", rt, msg.c_str());

    char buf[100];
    rt = read(fd, buf, 100);
    DEBUGLOG("success read %d bytes, [%s]", rt, std::string(buf).c_str());

}

void test_tcp_client() {
    tinyRPC::IPNetAddr::s_ptr addr = std::make_shared<tinyRPC::IPNetAddr>("127.0.0.1", 12345);
    tinyRPC::TcpClient client(addr);
    client.connect([addr, &client](){
        DEBUGLOG("connect to [%s] success", addr->toString().c_str());
        std::shared_ptr<tinyRPC::StringProtocol> message = std::make_shared<tinyRPC::StringProtocol>();
        message->info = "hello tinyRPC";
        std::string msg = "123456";
        message->setReqId(msg);
        client.writeMessage(message, [](tinyRPC::AbstractProtocol::s_ptr msg_ptr) {
            DEBUGLOG("send message success");
        });

        client.readMessage("123456", [](tinyRPC::AbstractProtocol::s_ptr msg_ptr) {
            std::shared_ptr<tinyRPC::StringProtocol> message = std::dynamic_pointer_cast<tinyRPC::StringProtocol>(msg_ptr);
            DEBUGLOG("req_id[%s], get response %s", message->getReqId().c_str(), message->info.c_str());
        });
    });
}

int main() {

    tinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyRPC::Logger::InitGlobalLogger();

    // test_connect();

    test_tcp_client();
    

    return 0;
}