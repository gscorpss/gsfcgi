#ifndef STREAMBUFFER_H
#define STREAMBUFFER_H
#include <stddef.h>

namespace sockpp
{

class StreamBuffer
{
    const size_t blockSize = 2048;
public:
    StreamBuffer(size_t sz = 0);
    StreamBuffer(StreamBuffer&& _buffer)
    : outOffset(_buffer.outOffset)
    , inOffset(_buffer.inOffset)
    , size(_buffer.size)
    , buffer(_buffer.buffer)
    {
        _buffer.outOffset = _buffer.inOffset = _buffer.size = 0;
        _buffer.buffer = nullptr;
    }
    ~StreamBuffer();

    void clear() { outOffset = inOffset = 0; }
    void* reserv(size_t sz);

    void* writablePtr() { return buffer + inOffset; }
    size_t writableSize() const { return size - inOffset; }
    void confirmWrite(size_t sz)
    {
        inOffset += sz;
        if (inOffset > size)
            inOffset = size;
    }

    void* data() { return buffer + outOffset; }
    size_t dataSize() const { return inOffset - outOffset; }
    void confirmReceived(size_t sz)
    {
        outOffset += sz;
        if (outOffset > size)
            outOffset = size;
    }
private:
    size_t outOffset = 0;
    size_t inOffset = 0;
    size_t size = 0;
    char* buffer = nullptr;
};

class InStreamBuffer
{
public:
    InStreamBuffer(StreamBuffer& _buffer) : buffer(_buffer) {}
    void* data() { return buffer.data(); }
    size_t dataSize() const { return buffer.dataSize(); }
    void confirmReceived(size_t sz) { buffer.confirmReceived(sz); }
private:
    StreamBuffer& buffer;
};

class OutStreamBuffer
{
public:
    OutStreamBuffer(StreamBuffer& _buffer) : buffer(_buffer) {}
    void* reserv(size_t sz) { return buffer.reserv(sz); }
    void* writablePtr() { return buffer.writablePtr(); }
    size_t writableSize() const { return buffer.writableSize(); }
    void confirm(size_t sz) { buffer.confirmWrite(sz); }
private:
    StreamBuffer& buffer;
};

} // namespace sockpp

#endif //STREAMBUFFER_H