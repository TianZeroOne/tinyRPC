#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <memory>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/config.h"
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/timer_event.h"
#include "tinyRPC/net/io_thread.h"
#include "tinyRPC/net/io_thread_group.h"

void test_io_thread() {
    
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rt != 0) {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd, 100);
    if (rt != 0) {
        ERRORLOG("listen error");
        exit(1);
    }

    tinyRPC::FdEvent event(listenfd);
    event.listen(tinyRPC::FdEvent::IN_EVENT, [listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);

        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });

    int i = 0;
    tinyRPC::TimerEvent::s_ptr timer_event = std::make_shared<tinyRPC::TimerEvent>(
        1000, true, [&i]() {
            INFOLOG("trigger timer event, count=%d", i++);
        }
    );

    // tinyRPC::IOThread io_thread;

    // io_thread.getEventLoop()->addEpollEvent(&event);
    // io_thread.getEventLoop()->addTimerEvent(timer_event);
    // io_thread.start();

    // io_thread.join();

    tinyRPC::IOThreadGroup io_thread_group(2);
    tinyRPC::IOThread* io_thread = io_thread_group.getIOThread();
    io_thread->getEventLoop()->addEpollEvent(&event);
    io_thread->getEventLoop()->addTimerEvent(timer_event);

    tinyRPC::IOThread* io_thread2 = io_thread_group.getIOThread();
    io_thread2->getEventLoop()->addEpollEvent(&event);

    io_thread_group.start();
    io_thread_group.join();


}

int main() {

    tinyRPC::Config::SetGlobalConfig("../conf/tinyrpc.xml");
    tinyRPC::Logger::InitGlobalLogger();

    test_io_thread();
    
    return 0;
}
void test() {
    tinyRPC::EventLoop* eventloop = new tinyRPC::EventLoop();

    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1) {
        ERRORLOG("listenfd = -1");
        exit(0);
    }

    sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_port = htons(12345);
    addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &addr.sin_addr);

    int rt = bind(listenfd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    if (rt != 0) {
        ERRORLOG("bind error");
        exit(1);
    }

    rt = listen(listenfd, 100);
    if (rt == -1) {
        ERRORLOG("listen error");
        exit(1);
    }

    tinyRPC::FdEvent event(listenfd);
    event.listen(tinyRPC::FdEvent::IN_EVENT, [listenfd](){
        sockaddr_in peer_addr;
        socklen_t addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));
        int clientfd = accept(listenfd, reinterpret_cast<sockaddr*>(&peer_addr), &addr_len);
        
        DEBUGLOG("success get client fd[%d], peer addr: [%s:%d]", clientfd, inet_ntoa(peer_addr.sin_addr), ntohs(peer_addr.sin_port));
    });

    eventloop->addEpollEvent(&event);

    int i = 0;
    tinyRPC::TimerEvent::s_ptr timer_event = std::make_shared<tinyRPC::TimerEvent>(
        1000, true, [&i]() {
            INFOLOG("trigger timer event, count=%d", i++);
        }
    );
    
    eventloop->addTimerEvent(timer_event);
    eventloop->loop();

}