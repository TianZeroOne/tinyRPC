#ifndef TINYRPC_NET_TCP_ABSTRACT_PROTOCOL_H
#define TINYRPC_NET_TCP_ABSTRACT_PROTOCOL_H

#include <memory>
#include "tinyRPC/net/tcp/tcp_buffer.h"
namespace tinyRPC {
class AbstractProtocol : public std::enable_shared_from_this<AbstractProtocol> {
    public:
        typedef std::shared_ptr<AbstractProtocol> s_ptr;
        std::string getReqId() {
            return m_req_id;
        }
        void setReqId(std::string& req_id) {
            m_req_id = req_id;
        }

        virtual ~AbstractProtocol() {}


    protected:
        std::string m_req_id; // 请求号，唯一标识

};
}

#endif