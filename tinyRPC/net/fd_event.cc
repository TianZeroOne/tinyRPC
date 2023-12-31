
#include <string.h>
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/comm/log.h"

namespace tinyRPC {
FdEvent::FdEvent(int fd) : m_fd(fd) {
    memset(&m_listen_events, 0, sizeof(m_listen_events)); // 初始化
}

FdEvent::~FdEvent() {

}
std::function<void()> FdEvent::handler(TriggerEvent event) {
    if (event == TriggerEvent::IN_EVENT) {
        return m_read_callback;
    } else {
        return m_write_callback;
    }
}

void FdEvent::listen(TriggerEvent event_type, std::function<void()> callback) {
    if (event_type == TriggerEvent::IN_EVENT) {
        m_listen_events.events |= EPOLLIN;
        m_read_callback = callback;   
    } else {
        m_listen_events.events |= EPOLLOUT;
        m_write_callback = callback;
    }
    m_listen_events.data.ptr = this; // 获取当前fd
}

}