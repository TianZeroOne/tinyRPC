#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include "tinyRPC/net/eventloop.h"
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/util.h"

#define ADD_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    int op = EPOLL_CTL_ADD; \
    if (it != m_listen_fds.end()) { \
        op = EPOLL_CTL_MOD; \
    } \
    epoll_event tmp = event->getEpollEvent(); \
    INFOLOG("epoll_event.events = %d", (int)tmp.events); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed epoll_ctl when add fd %d, errno=%d, error=%s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.insert(event->getFd()); \
    DEBUGLOG("add event success, fd[%d]", event->getFd()); \

#define DELETE_TO_EPOLL() \
    auto it = m_listen_fds.find(event->getFd()); \
    if (it == m_listen_fds.end()) { \
        return; \
    } \
    int op = EPOLL_CTL_DEL; \
    epoll_event tmp = event->getEpollEvent(); \
    int rt = epoll_ctl(m_epoll_fd, op, event->getFd(), &tmp); \
    if (rt == -1) { \
        ERRORLOG("failed epoll_ctl when delete fd %d, errno=%d, error=%s", event->getFd(), errno, strerror(errno)); \
    } \
    m_listen_fds.erase(event->getFd()); \
    DEBUGLOG("delete event success, fd[%d]", event->getFd()); \

namespace tinyRPC {
// thread_local 关键字修饰的变量具有线程周期，变量在线程开始的时候被生成，线程结束的时候被销毁
static thread_local EventLoop* t_current_eventloop = nullptr;
static int g_epoll_max_timeout = 10000;
static int g_epoll_max_events = 10;

EventLoop::EventLoop() {
    if (t_current_eventloop != nullptr) { // 代表创建过
        ERRORLOG("failed to create event loop, this thread has created event loop");
        exit(0);
    }

    m_thread_id = getThreadId();

    m_epoll_fd = epoll_create(10); // 这个值没啥用
    if (m_epoll_fd == -1) {
        ERRORLOG("failed to create event loop, epoll_create error, error info[%d]", errno);
        exit(0);
    }

    initWakeUpEvent();

    initTimer();

    INFOLOG("succ create event loop in thread %d", m_thread_id);
    t_current_eventloop = this;
}

EventLoop::~EventLoop() {
    close(m_epoll_fd);
    if (m_wakeup_fd_event) {
        delete m_wakeup_fd_event;
        m_wakeup_fd_event = NULL;
    }
    if (m_timer) {
        delete m_timer;
        m_timer = NULL;
    }
}

void EventLoop::initTimer() {
    m_timer = new Timer();
    addEpollEvent(m_timer);
}

void EventLoop::addTimerEvent(TimerEvent::s_ptr event) {
    m_timer->addTimerEvent(event);
}

void EventLoop::initWakeUpEvent() {
    m_wakeup_fd = eventfd(0, EFD_NONBLOCK); // 非阻塞
    if (m_wakeup_fd < 0) {
        ERRORLOG("failed to create event loop, eventfd create error, error info[%d]", errno);
        exit(0);
    }

    m_wakeup_fd_event = new WakeUpFdEvent(m_wakeup_fd);
    m_wakeup_fd_event->listen(FdEvent::IN_EVENT, [this]() {
        char buf[8];
        while (read(m_wakeup_fd, buf, 8) != -1 && errno != EAGAIN) {
        }
        DEBUGLOG("read full bytes from wakeup fd[%d]", m_wakeup_fd);
    });

    addEpollEvent(m_wakeup_fd_event);
}



void EventLoop::loop() {
    m_is_looping = true;
    while (!m_stop_flag) {
        ScopeMutex<Mutex> lock(m_mutex);
        std::queue<std::function<void()>> tmp_tasks;
        tmp_tasks.swap(m_pending_tasks);
        lock.unlock();

        while (!tmp_tasks.empty()) {
            std::function<void()> cb = tmp_tasks.front();
            tmp_tasks.pop();
            if (cb) {
                cb();
            }
        }

        // 如果有定时任务需要执行，那么执行
        // 1. 怎么判断一个定时任务需要执行？now() > TimerEvent.arrtive_time
        // 2. arrtive_time 如何让 eventloop 监听

        
        int timeout = g_epoll_max_timeout;
        epoll_event result_events[g_epoll_max_events];

        // DEBUGLOG("now begin to epoll_wait");
        int rt = epoll_wait(m_epoll_fd, result_events, g_epoll_max_events, timeout);
        DEBUGLOG("now end epoll_wait, rt = %d", rt);

        if (rt < 0) {
            ERRORLOG("epoll_wait error, errno=%d", errno)
        } else {
            for (int i = 0; i < rt; i++) {
                epoll_event trigger_event = result_events[i];
                FdEvent* fd_event = static_cast<FdEvent*>(trigger_event.data.ptr); // 强制类型转换
                if (fd_event == NULL) {
                    continue;
                }

                if (trigger_event.events & EPOLLIN) {
                    DEBUGLOG("fd %d trigger EPOLLIN event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::IN_EVENT));
                }
                if (trigger_event.events & EPOLLOUT) {
                    DEBUGLOG("fd %d trigger EPOLLOUT event", fd_event->getFd());
                    addTask(fd_event->handler(FdEvent::OUT_EVENT));
                }
            }
        }
    }

}

void EventLoop::wakeup() {
    INFOLOG("WAKE UP");
    m_wakeup_fd_event->wakeup();
}

void EventLoop::dealWakeup() {

}

void EventLoop::stop() {
    m_stop_flag = true;
}

void EventLoop::addEpollEvent(FdEvent* event) {
    if (isInLoopThread()) { // 代表是在当前线程
        ADD_TO_EPOLL();
    } else {
        auto cb = [this, event]() {
            ADD_TO_EPOLL();
        };
        addTask(cb, true);
    }
}
        
void EventLoop::deleteEpollEvent(FdEvent* event) {
    if (isInLoopThread()) {
        DELETE_TO_EPOLL();
    } else {
        auto cb = [this, event]() {
            DELETE_TO_EPOLL();
        };
        addTask(cb, true);
    }
}

void EventLoop::addTask(std::function<void()> cb, bool is_wake_up) {
    ScopeMutex<Mutex> lock(m_mutex);
    m_pending_tasks.push(cb);
    lock.unlock();
    if (is_wake_up) {
        wakeup();
    }

}
bool EventLoop::isInLoopThread() { // 判断当前线程是否是IO线程
    return getThreadId() == m_thread_id;
}

EventLoop* EventLoop::GetCurrentEventLoop() {
    if (!t_current_eventloop) {
        t_current_eventloop = new EventLoop();
    }
    return t_current_eventloop;
}

bool EventLoop::isLooping() {
    return m_is_looping;
}
}