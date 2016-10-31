#include "IPv4Addr.h"
#include <stdexcept>
#include <system/Exceptions.h>
#include <sstream>

namespace sockpp
{

/**
 * IPv4Addr implementation
 **/
IPv4Addr::IPv4Addr(in_addr _netAddr) : netAddr(_netAddr)  {}

IPv4Addr::IPv4Addr(const char* addr) { init(addr); }

IPv4Addr::IPv4Addr(const std::string& addr) { init(addr.c_str()); }

in_addr IPv4Addr::getHostAddr() const
{
    in_addr addr;
    addr.s_addr = ntohl(netAddr.s_addr);
    return addr;
}

in_addr IPv4Addr::getNetAddr() const
{
    return netAddr;
}

IPv4Addr::operator std::string() const
{
    return inet_ntoa(getNetAddr());
}

void IPv4Addr::init(const char* str)
{
    if (!inet_aton(str, &netAddr))
    {
        std::stringstream stream;
        stream << '\'' << str << "' is not valid IPv4 address";
        throw gssystem::Exception(stream.str());
    }
}

} // namespace sockpp
