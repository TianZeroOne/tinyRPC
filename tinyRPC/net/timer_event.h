#ifndef TINYRPC_NET_TIMEREVENT_H
#define TINYRPC_NET_TIMEREVENT_H
#include <functional>
#include <memory>

namespace tinyRPC {

class TimerEvent {
    public:
        typedef std::shared_ptr<TimerEvent> s_ptr;

        TimerEvent(int interval, bool is_repeated, std::function<void()> cb);

        int64_t getArriveTime() {
            return m_arrive_time;
        }
        void setCancled(bool value) {
            m_is_cancled = value;
        }
        bool isCancled() {
            return m_is_cancled;
        }
        bool isRepeated() {
            return m_is_repeated;
        }
        std::function<void()> getCallBack() {
            return m_task;
        }
        void resetTime();
        
    private:
        int64_t m_arrive_time;
        int64_t m_interval;
        bool m_is_repeated {false};
        bool m_is_cancled {false};

        std::function<void()> m_task;
};

}

#endif