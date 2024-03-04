#ifndef TINYRPC_NET_FD_EVENT_GROUP_H
#define TINYRPC_NET_FD_EVENT_GROUP_H

#include <vector>
#include "tinyRPC/comm/mutex.h"
#include "tinyRPC/net/fd_event.h"

// 创建 fd_event_group 便于存取
namespace tinyRPC {

class FdEventGroup {
    public:
        FdEventGroup(int size);
        ~FdEventGroup();
        FdEvent* getFdEvent(int fd);
    public:
        static FdEventGroup* GetFdEventGroup();

    private:
        int m_size {0};
        std::vector<FdEvent*> m_fd_group;
        Mutex m_mutex;

};

}
#endif