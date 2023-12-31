#include <stdio.h>
#include <sstream>
#include <sys/time.h>
#include "tinyRPC/comm/log.h"
#include "tinyRPC/comm/util.h"


    
namespace tinyRPC {

static Logger* g_logger = nullptr;

Logger* Logger::GetGlobalLogger() { // 单例模式中的懒汉式
    return g_logger;
}
void Logger::InitGlobalLogger() {
    LogLevel global_log_level = StringToLogLevel(Config::GetGlobalConfig()->get_log_level());
    printf("Init log level [%s]\n", LogLevelToString(global_log_level).c_str());
    g_logger = new Logger(global_log_level);
}

std::string LogLevelToString(LogLevel level) {
    switch (level)
    {
    case Debug:
        return "DEBUG";
    
    case Info:
        return "INFO";
    case Error:
        return "ERROR";
    
    default:
        return "UNKNOWN";
    }
}
LogLevel StringToLogLevel(const std::string log_level) {
    if (log_level == "DEBUG") {
        return Debug;
    } else if (log_level == "INFO") {
        return Info;
    } else if (log_level == "ERROR") {
        return Error;
    } else {
        return Unknown;
    }
}

std::string LogEvent::toString() {
    struct timeval now_time;

    gettimeofday(&now_time, nullptr); // 获得一天的秒数和us数

    struct tm now_time_t;
    localtime_r(&(now_time.tv_sec), &now_time_t);

    char buf[128];
    strftime(&buf[0], 128, "%y-%m-%d %H:%M:%S", &now_time_t);
    std::string time_str(buf);

    int ms = now_time.tv_usec / 1000;
    time_str = time_str + "." + std::to_string(ms);

    m_pid = getPid();
    m_thread_id = getThreadId();

    std::stringstream ss;

    ss << "[" << LogLevelToString(m_level) << "]\t"
        << "[" << time_str << "]\t"
        << "[" << m_pid << ":" << m_thread_id << "]\t";

    return ss.str();
}



void Logger::pushLog(const std::string& msg) {
    ScopeMutex<Mutex> lock(m_mutex); // 加锁
    m_buffer.push(msg);
    lock.unlock();
}

void Logger::log() {
    ScopeMutex<Mutex> lock(m_mutex); // 加锁
    std::queue<std::string> tmp; // 不能直接赋值
    m_buffer.swap(tmp);
    lock.unlock();

    while (!tmp.empty()) {
        std::string msg = tmp.front();
        tmp.pop();

        printf(msg.c_str());
    }
    
}

}