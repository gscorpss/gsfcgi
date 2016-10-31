#ifndef CORE_SOCKETS_H
#define CORE_SOCKETS_H

#include "NetEnums.h"

namespace sockpp
{

class Socket
{
public:
    Socket(int fd);
    Socket(Socket&& sock);
    Socket(const Socket&) = delete;

    virtual ~Socket();

    template<class Operation>
    Operation operation() { Operation o; o.setFd(fd); return o; }

    bool isValid() const;
private:
    int fd;
};

class NetSocket : public Socket
{
public:
    NetSocket(ProtocolFamilyEnum family, ProtocolTypeEnum type);
    NetSocket(int fd);
    NetSocket(NetSocket&& sock);
private:
};

} // namespace sockpp

#endif // CORE_SOCKETS_H
