#include "Sockets.h"
#include <StringStream.h>
#include <Exceptions.h>

#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>

namespace core
{
/**
 * IPv4Addr implementation
 **/
IPv4Addr::IPv4Addr(in_addr _hostAddr) : hostAddr(_hostAddr)  {}

IPv4Addr::IPv4Addr(const char* addr) { init(addr); }

IPv4Addr::IPv4Addr(const std::string& addr) { init(addr.c_str()); }

in_addr IPv4Addr::getHostAddr() const { return hostAddr; }

in_addr IPv4Addr::getNetAddr() const
{
    in_addr addr;
    addr.s_addr = htonl(hostAddr.s_addr);
    return addr;
}

void IPv4Addr::init(const char* str)
{
    if (!inet_aton(str, &hostAddr))
    {
        StringStream stream;
        stream << '\'' << str << "' is not valid";
        throw Exception(stream.c_str());
    }
}

/**
 * Socket class implementation
 **/
Socket::Socket(ProtocolFamily::Enum family, ProtocolType::Enum type)
: protocolFamily(family)
, protocolType(type)
{
    sock = socket(family, type, 0);
    if (sock < 0)
    {
        StringStream stream;
        stream << "Unable to create socket family=" << (int)family
            << " type=" << type << " with error";
        throw ErrnoException(stream.c_str());
    }
}

Socket::~Socket()
{
    if (sock >= 0)
    {
        close(sock);
    }
}


bool Socket::bind(uint16_t port)
{
    in_addr addr;
    addr.s_addr = INADDR_ANY;
    return bind(IPv4Addr(addr), port);
}

bool Socket::bind(const IPv4Addr& addr, uint16_t port)
{
    struct sockaddr_in address;
    address.sin_port = htons(port);
    address.sin_addr = addr.getNetAddr();
    address.sin_family = protocolFamily;
    if (!::bind ( sock, (sockaddr*)&address, sizeof( address ) ))
        return true;
    std::cerr << "Unable to bind socpet with error : (" << errno << ") " << strerror(errno) << std::endl;
    return false;
}

bool Socket::setNonBlocking()
{
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0)
    {
        std::cerr << "Unable to get flags : (" << errno << ") " << strerror(errno) << std::endl;
        return false;
    }
    flags |= O_NONBLOCK;
    if (fcntl(sock, F_SETFL, flags) == 0)
        return true;
    std::cerr << "Unable to set flags : (" << errno << ") " << strerror(errno) << std::endl;
    return false;
}


}