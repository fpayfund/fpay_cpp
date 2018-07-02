#pragma once

#include <stdexcept>
#include "ListBuffer.h"
#include "sockethelper.h"

namespace sox
{

//class buffer_overflow : public std::runtime_error
//{
//public:
//  buffer_overflow(const std::string& _w): std::runtime_error(_w) {}
//};
//
//class buffer_overflow_all : public buffer_overflow
//{
//public:
//  buffer_overflow_all(const std::string& _w): buffer_overflow(_w) {}
//};
//
//class FilterDefault
//{
//protected:
//  inline void filterRead(char *, size_t)  {}
//  inline char *filterWrite(char *data, size_t) {return data;}
//};

template<class BufferClass, class FilterClass = FilterDefault>
struct SockBufferO: public BufferClass, FilterClass
{
  using BufferClass::npos;
  using BufferClass::append;
  using BufferClass::empty;
  using BufferClass::erase;
  using BufferClass::size;
  using BufferClass::data;

  using FilterClass::filterWrite;

  void write(char* msg, size_t size)
  {
    if(size == 0)
      return;

    char *enc = filterWrite(msg, size);
    if(!append(enc, size))
      throw buffer_overflow_all("output buffer overflow [all]");
  }

  void write(Sockethelper & so, char * msg, size_t size)
  {
    if (size == 0)
      return;

    char *enc = filterWrite(msg, size);

    int nsent = 0;
    if (empty())
    {
		// 注意这里Sockethelper::send
		// a、返回0, (errno == EAGAIN || errno == EINTR || errno == EINPROGRESS)
		// b、抛异常 throw socket_error(errno, "send"),留给应用catch做回收
		// c、所以不会有-1返回
      nsent = so.send(enc, size);
    }

    if (!append(enc + nsent, size - nsent))
    {
      // all or part append error .
      if (nsent > 0)
        throw buffer_overflow("output buffer overflow");
      else 
        throw buffer_overflow_all("output buffer overflow [all]");
    }
  }

  int flush(Sockethelper & so, size_t n = npos)
  {
    size_t nflush = size();
    if (n < nflush)
      nflush = n;
    int ret = so.send(data(), nflush);
    erase(ret);

    return ret;
  }
};

}
