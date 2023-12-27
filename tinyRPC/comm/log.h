#ifndef TINYRPC_COMM_LOG_H
#define TINYRPC_COMM_LOG_H

#include <string>
#include <queue>
#include <memory>

#include "tinyRPC/comm/config.h"
#include "tinyRPC/comm/mutex.h"

namespace tinyRPC {


template<typename... Args>
std::string formatString(const char* str, Args&&... args) {
    
    int size = snprintf(nullptr, 0, str, args...); // 格式化输出字符串

    std::string result;
    if (size > 0) {
        result.resize(size);
        snprintf(&result[0], size + 1, str, args...);
    }
    return result;
}
// ##__VA_ARGS__代表可变参数的宏
#define DEBUGLOG(str, ...) \
    if (tinyRPC::Logger::GetGlobalLogger()->getLogLevel() <= tinyRPC::Debug) { \
        tinyRPC::Logger::GetGlobalLogger()->pushLog((new tinyRPC::LogEvent(tinyRPC::LogLevel::Debug))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
        tinyRPC::Logger::GetGlobalLogger()->log(); \
    } \
    

#define INFOLOG(str, ...) \
    if (tinyRPC::Logger::GetGlobalLogger()->getLogLevel() <= tinyRPC::Info) { \
        tinyRPC::Logger::GetGlobalLogger()->pushLog((new tinyRPC::LogEvent(tinyRPC::LogLevel::Info))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
        tinyRPC::Logger::GetGlobalLogger()->log(); \
    } \

#define ERRORLOG(str, ...) \
    if (tinyRPC::Logger::GetGlobalLogger()->getLogLevel() <= tinyRPC::Error) { \
        tinyRPC::Logger::GetGlobalLogger()->pushLog((new tinyRPC::LogEvent(tinyRPC::LogLevel::Error))->toString() + "[" + std::string(__FILE__) + ":" + std::to_string(__LINE__) + "]\t" + tinyRPC::formatString(str, ##__VA_ARGS__) + "\n"); \
        tinyRPC::Logger::GetGlobalLogger()->log(); \
    } \

enum LogLevel {
    Unknown = 0,
    Debug = 1,
    Info = 2,
    Error = 3
    
};

class Logger {
    public:
        typedef std::shared_ptr<Logger> s_ptr; // 共享指针，允许多个指针指向同一个对象
        void pushLog(const std::string& msg);
        void log();
        Logger(LogLevel level) : m_set_level(level) {};
        LogLevel getLogLevel() const {
            return m_set_level;
        }

    public:
        static Logger* GetGlobalLogger();
        static void InitGlobalLogger();

    private:
        LogLevel m_set_level;

        std::queue<std::string> m_buffer;

        Mutex m_mutex;


};

std::string LogLevelToString(LogLevel level);
LogLevel StringToLogLevel(const std::string log_level);

class LogEvent {
    public:
        LogEvent(LogLevel level) : m_level(level){};

        std::string getFileName() const { // const 防止被修改
            return m_file_name;
        }

        LogLevel getLogLevel() const {
            return m_level;
        }

        std::string toString();



    private:
        std::string m_file_name;  // 文件名
        std::string m_file_line;  // 行号
        int m_pid;
        int m_thread_id;
        std::string m_time;

        LogLevel m_level; // 日志等级
        Logger::s_ptr m_logger;
};



}
#endif // !TINYRPC_COMMON_LOG_H
