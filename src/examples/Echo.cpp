
#include <iostream>
#include <thread>
#include <vector>
#include <sockpp/Epoller.h>
#include <sockpp/Sockets.h>
#include <sockpp/SocketOperations.h>

#include <system/Exceptions.h>
#include <system/Signals.h>

bool stopping = false;

void pollmain(sockpp::Epoller& epoller)
{
    while(!stopping)
    {
        epoller.poll(1);
    }
}

class Connection
{
public:
private:
    sockpp::Socket socket;
};

class Listener : private sockpp::EpollSubscription
{
public:
    Listener()
    : socket(sockpp::ProtocolFamilyEnum::INET, sockpp::ProtocolTypeEnum::STREAM)
    {
        socket.operation<sockpp::operations::NonBlocking>().set(true);
        socket.operation<sockpp::operations::ReusePort>().turnOn();
    }

    bool start(short port, sockpp::Epoller epoller)
    {
        return socket.operation<sockpp::operations::Listen>().port(port)
            && epoller.subscribe(*this, socket, sockpp::EpollEventType::IN);
    }

    void stop();

private:
    virtual void onEvent (sockpp::EpollEventType event)
    {
        if (event != sockpp::EpollEventType::IN)
        {
            std::cerr << "unexpected event";
            return;
        }
        sockpp::Socket sock(socket.operation<sockpp::operations::Accept>().accept());
    }

    sockpp::NetSocket socket;
};

int main(int argc, char **argv)
{
    sockpp::Epoller epoller;
    std::thread th(pollmain, std::ref(epoller));

    gssystem::Signals signals({SIGINT});

    int signal = signals.wait();

    std::cout << "received signal=" << signal << std::endl;

    stopping = true;
    th.join();

//    std::cout << "Hello, world!" << std::endl;
    return 0;
}
