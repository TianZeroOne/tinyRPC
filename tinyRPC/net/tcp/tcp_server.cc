#include "tinyRPC/net/tcp/tcp_server.h"
#include "tinyRPC/comm/log.h"

namespace tinyRPC {
TcpServer::TcpServer(NetAddr::s_ptr local_addr) : m_local_addr(local_addr) {
    init();
    INFOLOG("tinyRPC TcpServer listen succ on [%s]", m_local_addr->toString().c_str());


}

TcpServer::~TcpServer() {
    if (m_main_event_loop) {
        delete m_main_event_loop;
        m_main_event_loop = nullptr;
    }
}

void TcpServer::init() {
    m_acceptor = std::make_shared<TcpAcceptor>(m_local_addr);

    m_main_event_loop = EventLoop::GetCurrentEventLoop();
    m_io_thread_group = new IOThreadGroup(2);

    m_listen_fd_event = new FdEvent(m_acceptor->getListFd()); // 使用成员变量保证生命周期
    m_listen_fd_event->listen(FdEvent::IN_EVENT, std::bind(&TcpServer::onAccept, this));

    m_main_event_loop->addEpollEvent(m_listen_fd_event);
}

void TcpServer::onAccept() {
    int client_fd = m_acceptor->accept();
    // FdEvent client_fd_event(client_fd);
    ++m_client_counts;

    // TODO：把clientfd添加到任意 IO 线程里面
    // m_io_thread_group->getIOThread()->getEventLoop()->addEpollEvent(&client_fd_event);
    INFOLOG("TcpServer succ get client, fd=%d", client_fd);

}

void TcpServer::start() {
    m_io_thread_group->start();
    m_main_event_loop->loop();
}
}