#ifndef GSFCGI_H
#define GSFCGI_H

namespace gsfcgi{

class FcgiRequest
{

};

class RequestListener
{
public:
    virtual ~RequestListener() {}
    virtual void onRequest(FcgiRequest& request) = 0;
};

} // namespace gsfcgi


#endif // GSFCGI_H

