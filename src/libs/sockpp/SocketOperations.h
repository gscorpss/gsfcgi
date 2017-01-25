#ifndef CORE_NET_SOCKET_OPERATIONS_H
#define CORE_NET_SOCKET_OPERATIONS_H

#include <stdlib.h>
#include "NetEnums.h"

namespace sockpp
{

class IPv4Addr;
class StreamBuffer;

namespace operations
{

class SocketGetter
{
public:
    SocketGetter(): fd(-1) {}
    void setFd(int _fd) { fd = _fd; }
    int getFd() { return fd; }
private:
    int fd;
};

class SocketOption : public SocketGetter
{
protected:
    template<class OptionValue>
    bool setOption(int level, int option, const OptionValue& optionValue, const char* optionName)
    {
        return setOptionImpl(getFd(), level, option, (void*)&optionValue, sizeof(optionValue), optionName);
    }
private:
    bool setOptionImpl(int fd, int level, int option, void* value, size_t valSize, const char* optionName);

};

struct Bind : public SocketGetter
{
    bool port(uint16_t port);
    bool addressPort(const IPv4Addr& addr, uint16_t port);
};

struct Listen : public SocketGetter
{
    bool listen();
    bool port(uint16_t port);
    bool addressPort(const IPv4Addr& addr, uint16_t port);

};

struct Connect : public SocketGetter
{
    bool connect(const IPv4Addr& addr, uint16_t port);
};

struct Data : public SocketGetter
{
    size_t sendTo(const IPv4Addr& addr, uint16_t port, void* buffer, size_t size);
    size_t send(void* buffer, size_t size);
    size_t send(StreamBuffer& buffer);

    size_t recv(void* buffer, size_t size);
    size_t recv(StreamBuffer& buffer);
};

struct Accept : public SocketGetter
{
    Socket accept();
};

struct SocketTraits : public SocketGetter
{
    SocketTraits() {}
    SocketTraits(int fd);
    ProtocolFamilyEnum getProtocolFamily();
    ProtocolTypeEnum getProtocolType();
};

struct NonBlocking : public SocketGetter
{
    bool isNonBlocking();
    bool setNonBlocking();
    bool setBlocking();
    bool set(bool nonBlocking);
};

struct Fragmentation : public SocketOption
{
    bool probe();
    bool allow();
    bool deny();
};

struct ReuseAddress : public SocketOption
{
    bool turnOn();
    bool turnOff();
};

struct ReusePort : public SocketOption
{
    bool turnOn();
    bool turnOff();
};

struct Broadcast : public SocketOption
{
    bool turnOn();
    bool turnOff();
};

struct Multicase : public SocketOption
{
    bool addMembership(const IPv4Addr& multiAddress, const IPv4Addr& selfAddress);
    bool dropMembership(const IPv4Addr& multiAddress, const IPv4Addr& selfAddress);
    bool setIpIface(const IPv4Addr& ifIp);
    bool setTTL(u_int8_t ttl);
};

} // namespace operations
} // namespace sockpp

#endif // CORE_NET_SOCKET_OPERATIONS_H
