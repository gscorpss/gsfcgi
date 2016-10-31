#include <sys/epoll.h>
#include <unistd.h>
#include <cstring>
#include <errno.h>
#include <iostream>
#include <array>
#include "Epoller.h"
#include "Sockets.h"
#include <system/Exceptions.h>
#include "SocketOperations.h"

const int MaxEvent = 100;

enum EpollCtlOperation : int
{
    Add = EPOLL_CTL_ADD,
    MOD = EPOLL_CTL_MOD,
    DEL = EPOLL_CTL_DEL
};

namespace sockpp
{

void EpollSubscription::unsubscribe()
{
    if (epoller->unsubscribe(*this))
        epoller->unsubscribe(*this);
}

Epoller::Epoller()
: epollHndl(epoll_create1(0))
{
    if (epollHndl < 0)
    {
        throw gssystem::ErrnoException("Unable to create epoll file descriptor");
    }
}

Epoller::~Epoller()
{
    close(epollHndl);
}

int Epoller::poll(int timeout)
{
    std::array<epoll_event, MaxEvent> events;
    int res = epoll_wait(epollHndl, events.data(), events.size(), timeout);
    if (res == -1)
    {
        if (errno == EINTR)
            return 0;

        std::cerr << "epoll wait error: (" << errno << ") " << strerror(errno) << std::endl;
        return res;
    }
    for(int i = 0; i < res; ++i)
    {
        reinterpret_cast<EpollSubscription*>(events[i].data.ptr)->onEvent((sockpp::EpollEventType)events[i].events);
    }
    return res;
}

bool Epoller::subscribe (EpollSubscription& subscription, Socket& socket, sockpp::EpollEventType events)
{
    epoll_event event;
    event.events = (unsigned int)events;
    event.data.ptr = &subscription;
    subscription.fd = socket.operation<operations::SocketGetter>().getFd();
    subscription.events = events;
    subscription.epoller = this;
    if (epoll_ctl(epollHndl, EpollCtlOperation::Add, socket.operation<operations::SocketGetter>().getFd(), &event))
    {
        std::cerr << "Unable to register epoll event with error: (" << errno << ") " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

bool Epoller::unsubscribe (EpollSubscription& subscription)
{
    if (!subscription.epoller)
        return false;

    if(epoll_ctl(epollHndl, EpollCtlOperation::DEL, subscription.fd, nullptr))
    {
        std::cerr << "Unable to remove subscription with error: (" << errno << ") " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}

} // namespace sockpp
