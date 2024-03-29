#include <unistd.h>
#include "tinyRPC/net/tcp/tcp_connection.h"
#include "tinyRPC/net/fd_event_group.h"
#include "tinyRPC/comm/log.h"
#include "tinyRPC/net/string_coder.h"

namespace tinyRPC {

TcpConnection::TcpConnection(EventLoop* event_loop, int fd, int buffer_size, NetAddr::s_ptr peer_addr, TcpConnectionType type) 
    : m_event_loop(event_loop), m_peer_addr(peer_addr), m_state(NotConnected), m_fd(fd), m_connection_type(type) {
    m_in_buffer = std::make_shared<TcpBuffer>(buffer_size);
    m_out_buffer = std::make_shared<TcpBuffer>(buffer_size);

    m_fd_event = FdEventGroup::GetFdEventGroup()->getFdEvent(fd);
    m_fd_event->setNonBlock();

    m_coder = new StringCoder();

    if (m_connection_type == TcpConnectionByServer) {
        listenRead();
    }

    
}

TcpConnection::~TcpConnection() {
    DEBUGLOG("~TcpConnection");
    if (m_coder) {
        delete m_coder;
        m_coder = nullptr;
    }
}

void TcpConnection::onRead() {
    // 1. 从 socket 缓冲区，调用系统的 read 函数读取字节 in_buffer 里面
    if (m_state != Conneted) {
        ERRORLOG("onRead error, client has already disconnected, addr[%s], clieantfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    bool is_read_all = false;
    bool is_close = false;
    while (!is_read_all) {
        if (m_in_buffer->writeAble() == 0) {
            m_in_buffer->resizeBuffer(2 * m_in_buffer->m_buffer.size());
        }
        int read_count = m_in_buffer->writeAble();
        int write_index = m_in_buffer->writeIndex();

        int rt = read(m_fd_event->getFd(), &(m_in_buffer->m_buffer[write_index]), read_count);
        DEBUGLOG("success read %d bytes from addr[%s], client fd[%d]", rt, m_peer_addr->toString().c_str(), m_fd);
        if (rt > 0) {
            m_in_buffer->moveWriteIndex(rt);
            if (rt == read_count) {
                continue;
            } else if (rt < read_count) {
                is_read_all = true;
                break;
            } 
        } else if (rt == 0) {
            is_close = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            is_read_all = true;
            break;
        }
    }

    if (is_close) {
        DEBUGLOG("peer closed, peer addr [%d], clientfd [%d]", m_peer_addr->toString().c_str(), m_fd);
        clear();
        return;
    }
    if (!is_read_all) {
        ERRORLOG("not read all data");
    }

    // TODO: 简单的 echo，后面补充 RPC 协议解析
    excute();
}

void TcpConnection::excute() {
    if (m_connection_type == TcpConnectionByServer) {
        // 将 RPC 请求执行业务逻辑，获取 RPC 响应，再把 RPC 响应发送回去
        std::vector<char> tmp;
        int size = m_in_buffer->readAble();
        tmp.resize(size);
        m_in_buffer->readFromBuffer(tmp, size);

        std::string msg;
        for (size_t i = 0; i < tmp.size(); ++i) {
            msg += tmp[i];
        }

        INFOLOG("success get request[%s] from client[%s]", msg.c_str(), m_peer_addr->toString().c_str());
        
        m_out_buffer->writeToBuffer(msg.c_str(), msg.length());

        listenWrite();
    } else {
        // 从 buffer 里面 decode 得到 message，执行其回调
        std::vector<AbstractProtocol::s_ptr> result;
        m_coder->decode(result, m_in_buffer);

        for (size_t i = 0; i < result.size(); ++i) {
            std::string req_id = result[i]->getReqId();
            auto it = m_read_dones.find(req_id);
            if (it != m_read_dones.end()) {
                it->second(result[i]);
            }
        }
    }
    
}

void TcpConnection::onWrite() {
    // 将当前的 out_buffer 里面的数据全部发送给 client
    if (m_state != Conneted) {
        ERRORLOG("onWrite error, client has already disconnected, addr[%s], clieantfd[%d]", m_peer_addr->toString().c_str(), m_fd);
        return;
    }

    if (m_connection_type == TcpConnectionByClient) {
        // 1. 将 message encode
        // 将数据写入 buffer 然后全部发送
        std::vector<AbstractProtocol::s_ptr> messages;
        for (size_t i = 0; i < m_write_dones.size(); ++i) {
            messages.push_back(m_write_dones[i].first);
        }
        m_coder->encode(messages, m_out_buffer);

    }

    bool is_write_all = false;
    while (true) {
        if (m_out_buffer->readAble() == 0) {
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        }
        int write_size = m_out_buffer->readAble();
        int read_index = m_out_buffer->readIndex();

        int rt = write(m_fd, &(m_out_buffer->m_buffer[read_index]), write_size);
        
        if (rt >= write_size) {
            DEBUGLOG("no data need to send to client [%s]", m_peer_addr->toString().c_str());
            is_write_all = true;
            break;
        } else if (rt == -1 && errno == EAGAIN) {
            // 发送缓冲区已满，不能再发送，等下次 fd 可写的时候再发送
            DEBUGLOG("write data error, errno==EAGIN and rt == -1");
            break;
        }
    }
    if (is_write_all) {
        m_fd_event->cancle(FdEvent::OUT_EVENT);
        m_event_loop->addEpollEvent(m_fd_event);
    }

    if (m_connection_type == TcpConnectionByClient) {
        for (size_t i = 0; i < m_write_dones.size(); ++i) {
            m_write_dones[i].second(m_write_dones[i].first);
        }
        m_write_dones.clear();
    }
    

}

void TcpConnection::setState(const TcpState state) {
    m_state = state;
}

TcpState TcpConnection::getState() {
    return m_state;
}

// 处理一些关闭连接后的清理动作
void TcpConnection::clear() {
    if (m_state == Closed) {
        return;
    }
    m_fd_event->cancle(FdEvent::IN_EVENT);
    m_fd_event->cancle(FdEvent::OUT_EVENT);

    m_event_loop->deleteEpollEvent(m_fd_event);
    m_state = Closed;
}

void TcpConnection::shutdown() {
    if (m_state == Closed || m_state == NotConnected) {
        return;
    }

    // 处于半关闭状态
    m_state = HalfClosing;
    // 调用 shutdown 关闭读写，意味着服务器不会再对这个 fd 进行读写操作
    // 发送 FIN 报文，触发了四次挥手的第一个阶段
    // 当 fd 发生可读事件，但是可读的数据为0，即对端发送了FIN
    ::shutdown(m_fd, SHUT_RDWR);
}

void TcpConnection::setConnectionType(TcpConnectionType type) {
    m_connection_type = type;
}

void TcpConnection::listenWrite() {
    m_fd_event->listen(FdEvent::OUT_EVENT, std::bind(&TcpConnection::onWrite, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::listenRead() {
    m_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpConnection::onRead, this));
    m_event_loop->addEpollEvent(m_fd_event);
}

void TcpConnection::pushSentMessage(AbstractProtocol::s_ptr message, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_write_dones.push_back(std::make_pair(message, done));
}
void TcpConnection::pushReadMessage(const std::string& req_id, std::function<void(AbstractProtocol::s_ptr)> done) {
    m_read_dones.insert(std::make_pair(req_id, done));
}
}