#include <fcntl.h>
#include <arpa/inet.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include "Sockets.h"
#include "IPv4Addr.h"
#include "SocketOperations.h"
#include "StreamBuffer.h"

namespace sockpp
{
namespace operations
{

bool SocketOption::setOptionImpl(int sock, int level, int option, void* value, size_t valSize, const char* optionName)
{
    if (!setsockopt (sock, level, option, value, valSize))
        return true;
    std::cerr << "Unable to set option '" << optionName << "' with error: (" << errno << ") " << strerror(errno) << std::endl;
    return false;
}

bool Bind::port(uint16_t port)
{
    in_addr addr;
    addr.s_addr = INADDR_ANY;
    return addressPort(IPv4Addr(addr), port);
}

bool Bind::addressPort(const IPv4Addr& addr, uint16_t port)
{
    SocketTraits f;
    f.setFd(getFd());
    struct sockaddr_in address;
    address.sin_port = htons(port);
    address.sin_addr = addr.getNetAddr();
    address.sin_family = (int)f.getProtocolFamily();
    if (!::bind ( getFd(), (sockaddr*)&address, sizeof( address ) ))
        return true;
    std::cerr << "Unable to bind socpet with error : (" << errno << ") " << strerror(errno) << std::endl;
    return false;
}

bool Listen::listen()
{
    if (!::listen(getFd(), SOMAXCONN))
        return true;
   //TODO: log error
    return false;
}

bool Listen::addressPort(const IPv4Addr& addr, uint16_t port)
{
    Bind b;
    b.setFd(getFd());
    return b.addressPort(addr, port) && listen();
}

bool Listen::port(uint16_t port)
{
    Bind b;
    b.setFd(getFd());
    return b.port(port) && listen();
}

bool Connect::connect(const IPv4Addr& addr, uint16_t port)
{
    sockaddr_in saddr;
    saddr.sin_addr = addr.getNetAddr();
    saddr.sin_port = htons(port);
    saddr.sin_family = (int)SocketTraits(getFd()).getProtocolFamily();
    if (!::connect(getFd(), (sockaddr*)&saddr, sizeof(saddr)))
    {
        return true;
    }
    //TODO: log error
    return false;
}

size_t Data::sendTo(const IPv4Addr& addr, uint16_t port, void* buffer, size_t size)
{
    sockaddr_in saddr;
    saddr.sin_family = (int)SocketTraits(getFd()).getProtocolFamily();
    saddr.sin_addr = addr.getNetAddr();
    saddr.sin_port = htons(port);
    size_t res = ::sendto(getFd(), buffer, size, 0, (sockaddr*)&saddr, sizeof(saddr));
    if (res != -1)
        return res;
    return false;
}

size_t Data::send(void* buffer, size_t size)
{
    return ::send(getFd(), buffer, size, 0);
}

size_t Data::send(StreamBuffer& buffer)
{
    size_t sz = send(buffer.data(), buffer.dataSize());
    if (sz > 0)
        buffer.confirmReceived(sz);
    return sz;
}

size_t Data::recv(void* buffer, size_t size)
{
    return ::recv(getFd(), buffer, size, 0);
}

size_t Data::recv(StreamBuffer& buffer)
{
    size_t sz = recv(buffer.writablePtr(), buffer.writableSize());
    if (sz > 0)
        buffer.confirmWrite(sz);
    return sz;
}

Socket Accept::accept()
{
    sockaddr_in saddr;
    socklen_t len = sizeof(saddr);
    Socket sock(::accept(getFd(), (sockaddr*)&saddr, &len));
    return std::move(sock);
}


SocketTraits::SocketTraits (int fd)
:SocketGetter()
{
    setFd(fd);
}


ProtocolFamilyEnum SocketTraits::getProtocolFamily()
{
   struct sockaddr sa;
   socklen_t len = sizeof(sa);
   if (getsockname(getFd(), &sa, &len))
       return ProtocolFamilyEnum::NOTDEFINED;
   //TODO: log error
   return (ProtocolFamilyEnum)sa.sa_family;
}

ProtocolTypeEnum SocketTraits::getProtocolType()
{
    int type;
    socklen_t length = sizeof(type);
    if (!getsockopt(getFd(), SOL_SOCKET, SO_TYPE, &type, &length) != 0)
        return ((ProtocolTypeEnum)type);
   //TODO: log error
    return ProtocolTypeEnum::NOTDEFINED;
}

bool NonBlocking::isNonBlocking()
{
    int flags = fcntl(getFd(), F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "Unable to get flags : (" << errno << ") " << strerror(errno) << std::endl;
        return false;
    }
    return flags & O_NONBLOCK;
}

bool NonBlocking::set (bool nonBlocking)
{
    int flags = fcntl(getFd(), F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "Unable to get flags : (" << errno << ") " << strerror(errno) << std::endl;
        return false;
    }
    if ((flags & O_NONBLOCK) == nonBlocking)
        return true;
    if (!nonBlocking)
    {
        flags &= O_NONBLOCK;
        flags %= O_NONBLOCK;
    }
    else
        flags |= O_NONBLOCK;
    if (fcntl(getFd(), F_SETFL, flags) == 0)
        return true;
    std::cerr << "Unable to set flags : (" << errno << ") " << strerror(errno) << std::endl;
    return false;
}

bool NonBlocking::setBlocking()
{
    return set(false);
}

bool NonBlocking::setNonBlocking()
{
    return set(true);
}


bool Fragmentation::probe()
{
    return this->setOption(SOL_IP, IP_MTU_DISCOVER, int(IP_PMTUDISC_PROBE), "probe ip packets fragmentation");
}

bool Fragmentation::allow()
{
    return this->setOption(SOL_IP, IP_MTU_DISCOVER, int(IP_PMTUDISC_WANT), "allow ip packets fragmentation");
}

bool Fragmentation::deny()
{
    return this->setOption(SOL_IP, IP_MTU_DISCOVER, int(IP_PMTUDISC_DO), "deny ip packets fragmentation");
}


bool ReuseAddress::turnOn()
{
    return this->setOption(SOL_SOCKET, SO_REUSEADDR, int(1), "turn on reuse address");
}

bool ReuseAddress::turnOff()
{
    return this->setOption(SOL_SOCKET, SO_REUSEADDR, int(0), "turn off reuse address");
}

bool ReusePort::turnOn()
{
    return this->setOption(SOL_SOCKET, SO_REUSEPORT, int(1), "turn on reuse port");
}

bool ReusePort::turnOff()
{
    return this->setOption(SOL_SOCKET, SO_REUSEPORT, int(0), "turn off reuse port");
}

bool Broadcast::turnOn()
{
    return this->setOption(SOL_SOCKET, SO_BROADCAST, int(1), "turn on broadcast");
}

bool Broadcast::turnOff()
{
    return this->setOption(SOL_SOCKET, SO_BROADCAST, int(0), "turn off broadcast");
}

bool Multicase::addMembership(const IPv4Addr& multiAddress, const IPv4Addr& selfAddress)
{
    ip_mreq mReq;
    mReq.imr_multiaddr = multiAddress.getHostAddr();
    mReq.imr_interface = selfAddress.getHostAddr();
    return this->setOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, mReq, "join multicast group");
}
bool Multicase::dropMembership(const IPv4Addr& multiAddress, const IPv4Addr& selfAddress)
{
    ip_mreq mReq;
    mReq.imr_multiaddr = multiAddress.getHostAddr();
    mReq.imr_interface = selfAddress.getHostAddr();
    return this->setOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, mReq, "leave multicast group");
}
bool Multicase::setIpIface(const IPv4Addr& ifIp)
{
    in_addr addr;
    addr = ifIp.getHostAddr();
    return this->setOption(IPPROTO_IP, IP_MULTICAST_IF, addr, "set multicast interface");
}
bool Multicase::setTTL(u_int8_t ttl)
{
    return this->setOption(IPPROTO_IP, IP_MULTICAST_TTL, ttl, "set multicast ttl");
}

} // namespace operations
} // namespace sockpp


