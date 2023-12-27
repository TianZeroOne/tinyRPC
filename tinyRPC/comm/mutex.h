#ifndef TINYRPC_COMM_MUTEX_H
#define TINYRPC_COMM_MUTEX_H

#include <pthread.h>
namespace tinyRPC {

template<class T> // 模板类
class ScopeMutex {
    public:
        ScopeMutex(T& mutex) : m_mutex(mutex) {
            mutex.lock();
            m_is_lock = true;
        }
        ~ScopeMutex() {
            m_mutex.unlock();
            m_is_lock = false;
        }
        void lock() {
            if (!m_is_lock) {
                m_mutex.lock();
            }
        }
        void unlock() {
            if (m_is_lock) {
                m_mutex.unlock();
            }
        }
    private:
        T& m_mutex;
        bool m_is_lock {false};

};

class Mutex {
    public:
        Mutex() {
            pthread_mutex_init(&m_mutex, NULL);
        }

        ~Mutex() {
            pthread_mutex_destroy(&m_mutex);
        }
        void lock() {
            pthread_mutex_lock(&m_mutex);
        }
        void unlock() {
            pthread_mutex_unlock(&m_mutex);
        }

    private:
        pthread_mutex_t m_mutex;

};

}

#endif