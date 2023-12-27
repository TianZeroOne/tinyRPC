#ifndef TINYRPC_COMM_CONFIG_H
#define TINYRPC_COMM_CONFIG_H
#include <map>

namespace tinyRPC {

class Config
{
    private:
        std::string m_log_level;
    public:
        static Config* GetGlobalConfig();
        static void SetGlobalConfig(const char* xmlfile);
    public:
        std::string get_log_level () {
            return m_log_level;
        }
        Config(const char* xmlfile);
    
};


}

#endif