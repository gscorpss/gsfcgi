#include "Sockets.h"
#include <system/Exceptions.h>
#include <stdexcept>
#include <iostream>
#include <fcntl.h>
#include <cstring>
#include <unistd.h>
#include <errno.h>
#include <sstream>

namespace sockpp
{

/**
 * Socket class implementation
 **/

Socket::Socket (int fd)
: fd(fd)
{
}

Socket::Socket(Socket&& sock)
: fd(sock.fd)
{
    sock.fd = -1;
}

Socket::~Socket()
{
    if (fd >= 0)
    {
        close(fd);
    }
}

bool Socket::isValid() const
{
    return fd >= 0;
}

NetSocket::NetSocket(ProtocolFamilyEnum family, ProtocolTypeEnum type)
: Socket(socket((int)family, (int)type, 0))
{
    if (!isValid())
    {
        std::stringstream stream;
        stream << "Unable to create socket family=" << (int)family
            << " type=" << (int)type;
        throw gssystem::ErrnoException(stream.str());
    }
}

NetSocket::NetSocket(NetSocket&& sock)
: Socket(std::move(sock))
{

}


} // namespace sockpp
