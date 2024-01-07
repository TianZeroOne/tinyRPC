#include <unistd.h>
#include "tinyRPC/net/wakeup_fd_event.h"
#include "tinyRPC/comm/log.h"

namespace tinyRPC {
WakeUpFdEvent::WakeUpFdEvent(int fd) : FdEvent(fd){
}

WakeUpFdEvent::~WakeUpFdEvent() {

}



void WakeUpFdEvent::wakeup() {
    char buf[8] = {'a'};
    int rt = write(m_fd, buf, 8);
    if (rt != 8) {
        ERRORLOG("write to wakeup fd less than 8 bytes, fd[%s]", m_fd);
    }
}
} // namespace tinyRPC
