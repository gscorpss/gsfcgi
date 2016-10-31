#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <exception>
#include <string>

namespace gssystem
{

class StringStream;

class BackTracer
{
public:
    BackTracer();
    virtual ~BackTracer();
    std::string getBacktrace() const;
private:
    void demangle(std::ostream& stream, char* str, int n) const;
    char **btStrings;
    int nptrs;
};

class Exception : public BackTracer, public std::exception
{
public:
    Exception(const std::string& _msg);

    virtual const char* what() const throw();
private:
    std::string msg;
};

class ErrnoException: public Exception
{
public:
    ErrnoException(const std::string& _msg);
};

} // namespace gssystem

#endif //EXCEPTIONS_H

