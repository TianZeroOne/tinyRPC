#ifndef TINYRPC_NET_EVENTLOOP_H
#define TINYRPC_NET_EVENTLOOP_H
#include <pthread.h>
#include <set>
#include <functional>
#include <queue>
#include "tinyRPC/comm/mutex.h"
#include "tinyRPC/net/fd_event.h"
#include "tinyRPC/net/wakeup_fd_event.h"
#include "tinyRPC/net/timer.h"

namespace tinyRPC {
class EventLoop {
    public:
        EventLoop();

        ~EventLoop();

        void loop();

        void wakeup();

        void stop();

        void addEpollEvent(FdEvent* event);

        void deleteEpollEvent(FdEvent* event);

        bool isInLoopThread();

        void addTask(std::function<void()> cb, bool is_wake_up = false);

        void addTimerEvent(TimerEvent::s_ptr event);

        bool isLooping();
    
    public:
        static EventLoop* GetCurrentEventLoop();
    
    private:
        void dealWakeup();

        void initWakeUpEvent();

        void initTimer();


    private:
        pid_t m_thread_id {0};
        int m_epoll_fd {0};
        int m_wakeup_fd {0};

        WakeUpFdEvent* m_wakeup_fd_event {NULL};

        bool m_stop_flag {false};
        std::set<int> m_listen_fds; // 监听的套接字

        std::queue<std::function<void()>> m_pending_tasks;

        Mutex m_mutex;

        Timer* m_timer {NULL};

        bool m_is_looping {false};

};

}

#endif