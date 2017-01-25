#include "gsfcgi.h"

namespace
{

const std::string EmptyStr;

enum class FcgiMessageType : unsigned char
{
    FCGI_BEGIN_REQUEST = 1,
    FCGI_ABORT_REQUEST = 2,
    FCGI_END_REQUEST = 3,
    FCGI_PARAMS = 4,
    FCGI_STDIN = 5,
    FCGI_STDOUT = 6,
    FCGI_STDERR = 7,
    FCGI_DATA = 8,
    FCGI_GET_VALUES = 9,
    FCGI_GET_VALUES_RESULT = 10,
    FCGI_UNKNOWN_TYPE = 11,
    FCGI_MAXTYPE = FCGI_UNKNOWN_TYPE
};

class FcgiHeader
{
    struct NetHeader
    {
        uint8_t version;
        FcgiMessageType type;
        uint8_t requestIdB1;
        uint8_t requestIdB0;
        uint8_t contentLengthB1;
        uint8_t contentLengthB0;
        uint8_t paddingLength;
        uint8_t reserved;
    };

public:
    FcgiHeader(sockpp::InStreamBuffer& buffer)
    {
        if (buffer.dataSize() < sizeof(NetHeader))
            return;

        NetHeader* head = reinterpret_cast<NetHeader*>(buffer.data());
        contentLength = (size_t)head->contentLengthB1 << 8 + head->contentLengthB0;

        if (buffer.dataSize() < contentLength + sizeof(NetHeader))
            return;

        requestId = (uint16_t)head->requestIdB1 << 8 + head->requestIdB0;
        type = head->type;
        finished = true;
    }
    bool messageFinished() const { return finished; }
    size_t getLength() const { return contentLength + padding; }
    size_t getContentLength() const { return contentLength; }
    uint16_t getRequestId() const { return requestId; }
    FcgiMessageType getType() const { return type; }
private:
    FcgiMessageType type;
    size_t contentLength;
    uint16_t requestId;
    uint8_t padding;
    bool finished = false;;
};

enum class FcgiRoleType : uint16_t
{
    FCGI_RESPONDER = 0x100,
    FCGI_AUTHORIZER = 0x200,
    FCGI_FILTER = 0x300,
};

enum class FcgiFlagType : uint8_t
{
    FCGI_KEEP_CONN = 1
};

struct FcgiBeginRequest
{
    FcgiRoleType role;
    FcgiFlagType flags;
    unsigned char reserved[5];
};

class RequestPatam
{
//    size_t readSize(ptr*&)

public:
    size_t parse(size_t size, sockpp::InStreamBuffer& buffer)
    {
        char* ptr = (char*)buffer.data();
        if (!nameSize)
        {
            do
            {
                nameSize += (unsigned char)(*ptr);
                --size;
            }
            while (*(ptr++) & 0x80);
            do
            {
                value += *reinterpret_cast<unsigned char*>(ptr);
                --size;
            }
            while (*(ptr++) & 0x80);
        }
        if (size && nameSize != name.size())
        {
            size_t szToCopy = std::min(size, nameSize - name.size());
            name.append(ptr, szToCopy);
            ptr += szToCopy;
            size -= szToCopy;
        }
        if (size && valueSize != value.size())
        {
            size_t szToCopy = std::min(size, valueSize - value.size());
            value.append(ptr, szToCopy);
            ptr += szToCopy;
            size -= szToCopy;
        }
        finished = (nameSize == name.size() && valueSize == value.size());
        return 0;
    }

    void clear()
    {
        nameSize = valueSize = 0;
        name.clear();
        value.clear();
        finished = false;
    }
    bool isFinished() { return finished; }
private:
    size_t nameSize = 0;
    size_t valueSize = 0;
    std::string name;
    std::string value;
    bool finished = false;
};

} // namespace anonymous

namespace gsfcgi
{

struct FcgiRequest::Param
{
    std::string name;
    std::string value;
};

class RequestParser::FcgiRequestImpl
{
    enum RequestStatus
    {
        Empty = 0,
        ParamsReading,
        StdInReading,
        Received,
        Error
    };
public:
    FcgiRequestImpl(RequestParser& _parser) : parser(_parser) {}

    bool onBeginRequest(FcgiHeader& head, sockpp::InStreamBuffer& buffer)
    {
        if (status == Empty)
        {
            parser.lsnr->onProtocolError("Request is not finished");
            return true;
        }

        FcgiBeginRequest* beginReq = reinterpret_cast<FcgiBeginRequest*>(buffer.data());

        role = beginReq->role;
        flags = beginReq->flags;
        status = ParamsReading;

        return true;
    }

    bool onParamsMsg(FcgiHeader& head, sockpp::InStreamBuffer& buffer)
    {
        return true;
    }
    bool onStdInMsg(FcgiHeader& head, sockpp::InStreamBuffer& buffer)
    {
        return true;
    }
private:
    FcgiRoleType role;
    FcgiFlagType flags;
    RequestStatus status = Empty;
    std::vector<RequestPatam*> paramsCache;
    RequestParser& parser;
};


void FcgiRequest::setResultCode(uint64_t resultCode)
{
}

const std::string & FcgiRequest::getAttribute(const std::string& name)
{
    for(Param* param : params)
    {
        if (param->name == name)
            return param->value;
    }
    return EmptyStr;
}


/**
 *  RequestParser implementation
 **/
RequestParser::RequestParser (RequestListener* _lsnr)
: lsnr(_lsnr)
{
}

sockpp::InStreamBuffer& RequestParser::operator<<(sockpp::InStreamBuffer& buffer)
{
    FcgiHeader header(buffer);
    if (!header.messageFinished())
        return buffer;

    FcgiRequestImpl& request =getRequest(header.getLength());

    buffer.confirmReceived(sizeof(FcgiHeader));

    switch(header.getType())
    {
        case FcgiMessageType::FCGI_BEGIN_REQUEST:
            request.onBeginRequest(header, buffer);
            break;
        case FcgiMessageType::FCGI_PARAMS:
            request.onParamsMsg(header, buffer);
            break;
        case FcgiMessageType::FCGI_STDIN:
            request.onStdInMsg(header, buffer);
            break;
        default:
            // TODO: log not supported message type;
            break;
    };

    buffer.confirmReceived(header.getLength());

    return buffer;
}

RequestParser::FcgiRequestImpl& RequestParser::getRequest(uint16_t requestId)
{
    FcgiRequestImpl* req = requests[requestId];
    if (!req)
        req = new FcgiRequestImpl(*this);
    return *req;
}


}
