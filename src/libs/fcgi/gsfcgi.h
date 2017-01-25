#ifndef GSFCGI_H
#define GSFCGI_H

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <sockpp/StreamBuffer.h>

namespace gsfcgi{

class FcgiRequest
{
protected:
    struct Param;
public:
    typedef std::unique_ptr<FcgiRequest> Ptr;
    void setResultCode(uint64_t resultCode = 0);
    const std::string& getAttribute(const std::string& name);
    sockpp::InStreamBuffer in() { return sockpp::InStreamBuffer(inBuffer); }
    sockpp::OutStreamBuffer out() { return sockpp::OutStreamBuffer(outBuffer); }
    sockpp::OutStreamBuffer err() { return sockpp::OutStreamBuffer(errBuffer); }
protected:
    virtual ~FcgiRequest() {}
    std::vector<Param*> params;
    sockpp::StreamBuffer inBuffer;
    sockpp::StreamBuffer outBuffer;
    sockpp::StreamBuffer errBuffer;
};

class RequestListener
{
public:
    virtual ~RequestListener() {}
    virtual void onRequest(FcgiRequest::Ptr&& request) = 0;
    virtual void onProtocolError(const std::string& error) = 0;
};

class RequestParser
{
    class FcgiRequestImpl;
    typedef std::map<uint16_t, FcgiRequestImpl*> Requests;
public:
    RequestParser(RequestListener* lsnr);

    sockpp::InStreamBuffer& operator<<(sockpp::InStreamBuffer& buffer);
private:
    FcgiRequestImpl& getRequest(uint16_t requestId);

    Requests requests;
    RequestListener* lsnr;
};


} // namespace gsfcgi


#endif // GSFCGI_H
