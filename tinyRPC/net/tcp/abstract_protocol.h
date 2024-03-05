#ifndef TINYRPC_NET_TCP_ABSTRACT_PROTOCOL_H
#define TINYRPC_NET_TCP_ABSTRACT_PROTOCOL_H

#include <memory>
namespace tinyRPC {
class AbstractProtocol {
    public:
        typedef std::shared_ptr<AbstractProtocol> s_ptr;

};
}

#endif