#ifndef TINYRPC_NET_TCP_ABSTRACT_CODER_H
#define TINYRPC_NET_TCP_ABSTRACT_CODER_H

#include <vector>
#include "tinyRPC/net/tcp/abstract_protocol.h"
#include "tinyRPC/net/tcp/tcp_buffer.h"
namespace tinyRPC {
class AbstractCoder {
    public:
        virtual void encode(std::vector<AbstractProtocol::s_ptr>& messages, TcpBuffer::s_ptr out_buffer) = 0;
        virtual void decode(std::vector<AbstractProtocol::s_ptr>& out_messages, TcpBuffer::s_ptr buffer) = 0;

        virtual ~AbstractCoder() {}
};
}
#endif