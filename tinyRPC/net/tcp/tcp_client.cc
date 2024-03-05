#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/net/tcp/tcp_client.h"
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/net/fd_event_group.h"

namespace tinyRPC {

TcpClient::TcpClient(NetAddr::s_ptr peer_addr) : m_peer_addr(peer_addr) {
    m_event_loop = EventLoop::GetCurrentEventLoop();
    m_fd = socket(peer_addr->getFamily(), SOCK_STREAM, 0);

    if (m_fd < 0) {
        ERRORLOG("TcpClient::TcpClient() error, failed to create fd");
        return;
    }

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(m_fd);
    m_fd_event->setNonBlock();

    m_connection = std::make_shared<TcpConnection>(m_event_loop, m_fd, 128, m_peer_addr);
    m_connection->setConnectionType(TcpConnectionByClient);

}
TcpClient::~TcpClient() {
    DEBUGLOG("TcpClient::~TcpClient()");
    if (m_fd > 0) {
        close(m_fd);
    }
}
void TcpClient::connect(std::function<void()> done) {
    int rt = ::connect(m_fd, m_peer_addr->getSockAddr(), m_peer_addr->getSockLen());
    if (rt == 0) {
        DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
        if (done) {
            done();
        }
    } else if (rt == -1) {
        if (errno == EINPROGRESS) {
            // epoll 监听可写事件，然后判断错误码
            m_fd_event->listen(FdEvent::OUT_EVENT, [this, done](){
                int error = 0;
                socklen_t error_len = sizeof(error);
                getsockopt(m_fd, SOL_SOCKET, SO_ERROR, &error, &error_len);
                if (error == 0) {
                    DEBUGLOG("connect [%s] success", m_peer_addr->toString().c_str());
                    if (done) {
                        done();
                    }
                } else {
                    ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
                }
                m_fd_event->cancle(FdEvent::OUT_EVENT); // 取消监听
                m_event_loop->addEpollEvent(m_fd_event);
            });
            m_event_loop->addEpollEvent(m_fd_event);

            if (!m_event_loop->isLooping()) { // 主线程创建了 event_loop 但可能未启动
                m_event_loop->loop();
            }
            
        } else {
            ERRORLOG("connect error, errno=%d, error=%s", errno, strerror(errno));
        }
    }
}

void TcpClient::writeMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done) {

}

void TcpClient::readMessage(AbstractProtocol::s_ptr request, std::function<void(AbstractProtocol::s_ptr)> done) {

}

}