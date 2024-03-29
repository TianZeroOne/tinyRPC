#ifndef TINYRPC_NET_IO_THREAD_GROUP_H
#define TINYRPC_NET_IO_THREAD_GROUP_H
#include <vector>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/net/io_thread.h"

namespace tinyRPC {

class IOThreadGroup {

    public:
        IOThreadGroup(int size);

        ~IOThreadGroup();

        void start();

        void join();

        IOThread* getIOThread();

    private:
        int m_size {0};

        std::vector<IOThread*> m_io_thread_group;

        int m_index {0};

};

}
#endif