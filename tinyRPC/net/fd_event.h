#ifndef TINYRPC_NET_FDEVENT_H
#define TINYRPC_NET_FDEVENT_H
#include <functional>
#include <sys/epoll.h>

namespace tinyRPC {
class FdEvent {
    public:
        enum TriggerEvent {
            IN_EVENT = EPOLLIN,
            OUT_EVENT = EPOLLOUT,
        };

        FdEvent(int fd);
        FdEvent();
        ~FdEvent();

        void setNonBlock();

        std::function<void()> handler(TriggerEvent event_type);

        void listen(TriggerEvent event_type, std::function<void()> callback);

        void cancle(TriggerEvent event_type);

        int getFd() const {
            return m_fd;
        }

        epoll_event getEpollEvent() {
            return m_listen_events;
        }


    protected:
        int m_fd {-1};

        epoll_event m_listen_events;

        std::function<void()> m_read_callback;
        std::function<void()> m_write_callback;

};
    
} // namespace name
#endif