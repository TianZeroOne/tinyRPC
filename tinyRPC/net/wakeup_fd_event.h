#ifndef TINYRPC_NET_WAKEUP_FDEVENT_H
#define TINYRPC_NET_WAKEUP_FDEVENT_H

#include "tinyRPC/net/fd_event.h"

namespace tinyRPC {
class WakeUpFdEvent : public FdEvent {

public:
    WakeUpFdEvent(int fd);

    ~WakeUpFdEvent();


    void wakeup();
private:

};


}

#endif