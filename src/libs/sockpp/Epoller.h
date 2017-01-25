#ifndef EPOLLER_h
#define EPOLLER_h
#include <sys/epoll.h>
#include "BitmaskOperations.h"

namespace sockpp
{

class Socket;

enum class EpollEventType : unsigned int
{
    EMPTY = 0,
    
    /// @brief The associated file is available for read(2) operations.
    IN = EPOLLIN,

    /// @brief The associated file is available for write(2) operations.
    OUT = EPOLLOUT,

    /** @brief Stream  socket peer closed connection, or shut down writing half of connection
     * (This flag is especially useful for writing simple code to detect peer shutdown when using
     * Edge Triggered monitoring.) */
    RDHUP = EPOLLRDHUP,

    /// @brief There is urgent data available for read(2) operations.
    PRI = EPOLLPRI,
    /** @brief Error condition happened on the associated file descriptor.  epoll_wait(2) will always wait
     * for this event; it is not necessary to set it in events. */
    ERR = EPOLLERR,

    /** @brief Hang up happened on the associated file descriptor.  epoll_wait(2) will always wait
     * for this event; it is not necessary to set it in events. */
    HUP = EPOLLHUP,

    /** @brief Sets the Edge Triggered behavior for the associated file descriptor.
     * The default behavior for epoll is Level Triggered.  See epoll(7) for more detailed information about Edge and
     * Level  Triggered event distribution architectures. */
    ET = EPOLLET
};

class Epoller;

class EpollSubscription
{
public:
    friend Epoller;

    virtual void onEvent(EpollEventType event) = 0;
    void unsubscribe();
private:
    Epoller* epoller = nullptr;
    sockpp::EpollEventType events;
    int fd = -1;
};

class Epoller
{
public:
    Epoller();
    ~Epoller();
    int poll(int timeout);
    bool subscribe(EpollSubscription& subscription, Socket& socket, EpollEventType events);
    bool unsubscribe(sockpp::EpollSubscription& subscription);
private:
    int epollHndl;
};

} // namespace sockpp

template<>
struct enable_bitmask_operators<sockpp::EpollEventType>
{
    static const bool enable=true;
};


#endif //EPOLLER_h
