
#include <iostream>
#include <thread>
#include <vector>
#include <sockpp/Epoller.h>
#include <sockpp/Sockets.h>
#include <sockpp/SocketOperations.h>
#include <sockpp/StreamBuffer.h>

#include <system/JobQueue.h>
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

class ConnectionListener
{
public:
    virtual ~ConnectionListener() {}
    virtual void onConnection(sockpp::Socket&& sock) = 0;
private:

};

class Listener : private sockpp::EpollSubscription
{
public:
    Listener(ConnectionListener& _connListener)
    : socket(sockpp::ProtocolFamilyEnum::INET, sockpp::ProtocolTypeEnum::STREAM)
    , connListener(_connListener)
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

        connListener.onConnection(std::move(socket.operation<sockpp::operations::Accept>().accept()));
    }

    sockpp::NetSocket socket;
    ConnectionListener& connListener;
};

class TcpConnection : private sockpp::EpollSubscription
{
    class ReadDataJob :gssystem::Job
    {
    public:
        ReadDataJob(TcpConnection& _connection)
        : connection(_connection)
        {}
        virtual void execute()
        {
            size_t sz =  sock.operation<sockpp::operations::Data>().recv(connection.inBuffer);
            if (sz > 0)
                connection.dataReceived(connection.inBuffer);
            else if (sz == -1)
            {
                //close connection
            }
        }
    private:
        TcpConnection& connection;
    };

public:
    TcpConnection(sockpp::Socket&& _sock, gssystem::JobQueue& _queue)
    : sock(std::move(_sock))
    , queue(_queue)
    {

    }

    ~TcpConnection()
    {}

    bool subscribe(sockpp::Epoller* epoller)
    {
        return epoller->subscribe(*this, sock, sockpp::EpollEventType::IN
                                               & sockpp::EpollEventType::OUT
                                               & sockpp::EpollEventType::RDHUP
                                               & sockpp::EpollEventType::HUP
                                               & sockpp::EpollEventType::ET);
    }
    virtual void dataReceived(sockpp::StreamBuffer& data)
    {
        
    }

    bool readable() const { return isReadable; }
    bool writable() const { return isWriteable; }
    bool isClosed() const { return closed; }
protected:

private:
    virtual void onEvent (sockpp::EpollEventType event)
    {
        if ((event & sockpp::EpollEventType::IN) != sockpp::EpollEventType::EMPTY)
        {
            isReadable = true;
        }
        if ((event & sockpp::EpollEventType::OUT) != sockpp::EpollEventType::EMPTY)
            isWriteable = true;
        if ((event & (sockpp::EpollEventType::HUP | sockpp::EpollEventType::RDHUP)) != sockpp::EpollEventType::EMPTY)
            closed = true;
    }
    sockpp::StreamBuffer inBuffer;
    sockpp::StreamBuffer outBuffer;
    sockpp::Socket sock;
    gssystem::JobQueue& queue;
    bool isReadable;
    bool isWriteable;
    bool closed = false;
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
