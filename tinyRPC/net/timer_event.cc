#include "tinyRPC/net/timer_event.h"
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/util.h"

namespace tinyRPC {

TimerEvent::TimerEvent(int interval, bool is_repeated, std::function<void()> cb) 
    : m_interval(interval), m_is_repeated(is_repeated), m_task(cb) {
    resetTime();
    

}
void TimerEvent::resetTime() {
    m_arrive_time = getNowMs() + m_interval;
    m_is_cancled = false;
    DEBUGLOG("success create timer event, will excute at [%lld]", m_arrive_time);
}

}