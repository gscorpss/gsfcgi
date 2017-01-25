#include "StreamBuffer.h"
#include <memory.h>

namespace sockpp
{

StreamBuffer::StreamBuffer (size_t sz)
: size (sz)
, buffer (sz ? new char[sz] : nullptr)
{
}

StreamBuffer::~StreamBuffer()
{
    if (buffer)
        delete buffer;
}

void* StreamBuffer::reserv (size_t sz)
{
    if (size - inOffset < sz)
    {
        if (size - dataSize() > sz)
        {
            memmove (data(), buffer, dataSize());
            inOffset = dataSize();
            outOffset = 0;
        }
        else
        {
            size_t newSize = ( ( (sz + inOffset - 1) / blockSize) + 1) * blockSize;
            char* newBuff = new char[newSize];
            memcpy (newBuff, buffer + outOffset, dataSize());
            delete [] buffer;
            buffer = newBuff;
            size = newSize;
            inOffset = dataSize();
            outOffset = 0;
        }
    }
    return buffer + inOffset;
}

} // namespace sockpp
