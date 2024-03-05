#ifndef TINYRPC_NET_TCP_TCP_CONNECTION_H
#define TINYRPC_NET_TCP_TCP_CONNECTION_H

#include <memory>
#include "tinyRPC/net/tcp/net_addr.h"
#include "tinyRPC/net/tcp/tcp_buffer.h"
#include "tinyRPC/net/io_thread.h"

namespace tinyRPC {
enum TcpState {
        NotConnected = 1,
        Conneted = 2,
        HalfClosing = 3, // 半连接
        Closed = 4,
};
enum TcpConnectionType {
    TcpConnectionByServer = 1, // 作为服务端使用，代表跟对端客户端的连接
    TcpConnectionByClient = 2, // 作为客户端使用，代表跟对端服务端的连接
};
class TcpConnection {

    public:
        typedef std::shared_ptr<TcpConnection> s_ptr;

    public:
        TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr);

        ~TcpConnection();

        void onRead();

        void excute();

        void onWrite();

        void setState(const TcpState state);

        TcpState getState();

        void clear();
        
        // 服务器主动关闭连接
        void shutdown();

        void setConnectionType(TcpConnectionType type);

    private:
        NetAddr::s_ptr m_local_addr;
        NetAddr::s_ptr m_peer_addr;

        TcpBuffer::s_ptr m_in_buffer; // 接受缓冲区
        TcpBuffer::s_ptr m_out_buffer; // 发送缓冲区

        EventLoop* m_event_loop;

        FdEvent* m_fd_event {NULL};

        TcpState m_state;

        int m_fd {0};

        TcpConnectionType m_connection_type {TcpConnectionByServer};
};

}
#endif