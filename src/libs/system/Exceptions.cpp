#include <execinfo.h>
#include <stdlib.h>
#include <cxxabi.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include <iostream>
#include "Exceptions.h"

namespace
{

std::string erronoString()
{
    char buffer[1024];
    strerror_r(errno, buffer, sizeof(buffer));
    std::stringstream stream;
    stream << '(' << errno << ") " << buffer;
    return stream.str();
}

} // namespace

namespace gssystem
{

BackTracer::BackTracer()
{
    void* buffer[1000];
    nptrs = ::backtrace(buffer, 1000);
    btStrings = backtrace_symbols(buffer, nptrs);
}
BackTracer::~BackTracer()
{
    free(btStrings);
}

std::string BackTracer::getBacktrace() const
{
    if (!btStrings)
    {
        return std::string("Backtrace not available");
    }
    std::stringstream stream;
    for (int i = 1 ; i < nptrs; ++i)
    {
        if (i > 1)
            stream << '\n';

        demangle(stream, btStrings[i], i);
    }
    return stream.str();
}

void BackTracer::demangle(std::ostream& stream, char* str, int n) const
{
    char* mangled_name = 0;
    char* offset_begin = 0;
    char* offset_end = 0;

    for (char* p = str; *p; ++p)
    {
        switch(*p)
        {
            case '(':
                mangled_name = p;
                continue;
            case '+':
                offset_begin = p;
                continue;
            case ')':
                offset_end = p;
                continue;
            default:
                break;
        }
    }
    if (mangled_name && offset_begin && offset_end && mangled_name < offset_begin)
    {
        *mangled_name++ = '\0';
        *offset_begin++ = '\0';
        *offset_end++ = '\0';

        int status;
        char * name = abi::__cxa_demangle(mangled_name, 0, 0, &status);

        if (status == 0)
        {
            stream << "[bt]: (" << n << ") " << str << " : " << name << "+" << offset_begin << offset_end;
        }
        else
        {
            stream  << "[bt]: (" << n << ") " << str << " : " << mangled_name << "+" << offset_begin << offset_end;
        }
        free(name);
    }
    else
    {
        stream << "[bt]: (" << n << ") " << str;
    }
}

Exception::Exception(const std::string& _msg)
: msg(_msg)
{
}

const char* Exception::what() const throw()
{
    return msg.c_str();
}

ErrnoException::ErrnoException(const std::string& _msg)
: Exception(_msg + ": " + erronoString())
{
}

} // namespace gssystem
