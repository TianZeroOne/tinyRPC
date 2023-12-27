#ifndef TINYRPC_COMM_UTIL_H
#define TINYRPC_COMM_UTIL_H
#include <sys/types.h>
#include <unistd.h>

namespace tinyRPC {

pid_t getPid();

pid_t getThreadId();

}

#endif