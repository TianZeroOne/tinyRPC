#ifndef TINYRPC_NET_TIMER_H
#define TINYRPC_NET_TIMER_H

#include <map>
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/timer_event.h"
#include "tinyRPC/comm/mutex.h"

namespace tinyRPC {

class Timer : public FdEvent {
    public:
        Timer();
        ~Timer();

        void addTimerEvent(TimerEvent::s_ptr event);

        void deleteTimerEvent(TimerEvent::s_ptr event);

        void onTimer(); // 当发生了 IO 事件，eventloop会调用
    private:
        void resetArriveTime();

    private:
        std::multimap<int64_t, TimerEvent::s_ptr> m_pending_events;
        Mutex m_mutex;
};

}

#endif