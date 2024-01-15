#ifndef TINYRPC_NET_TCP_TCP_SERVER_H
#define TINYRPC_NET_TCP_TCP_SERVER_H

#include "tinyRPC/net/tcp/tcp_acceptor.h"
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/io_thread_group.h"

namespace tinyRPC {
class TcpServer {
    public:
        TcpServer(NetAddr::s_ptr local_addr);

        ~TcpServer();

        void start();

    private:
        void init();
        // 当有新客户端连接之后需要执行
        void onAccept();

    private:
        TcpAcceptor::s_ptr m_acceptor;

        NetAddr::s_ptr m_local_addr; // 本地监听地址

        EventLoop* m_main_event_loop {nullptr};  // mainReactor

        IOThreadGroup* m_io_thread_group {nullptr};  // subReactor 组

        FdEvent* m_listen_fd_event;

        int m_client_counts {0};
};

}

#endif