
#ifndef __SNOX_SOCKBUFFER_H_INCLUDE__
#define __SNOX_SOCKBUFFER_H_INCLUDE__

#include <stdexcept>
#include "sockethelper.h"
#include "common/blockbuffer.h"

namespace sox
{

class buffer_overflow : public std::runtime_error
{
public:
	buffer_overflow(const std::string& _w): std::runtime_error(_w) {}
};

class buffer_overflow_all : public buffer_overflow
{
public:
	buffer_overflow_all(const std::string& _w): buffer_overflow(_w) {}
};

class FilterDefault
{
protected:
	void filterRead(char *, size_t)  {}
	char *filterWrite(char *data, size_t) {return data;}
};

template < class BlockBufferClass, class FilterClass = FilterDefault >
struct SockBuffer : public BlockBufferClass, public FilterClass
{
	using BlockBufferClass::npos;
	using BlockBufferClass::freespace;
	using BlockBufferClass::blocksize;
	using BlockBufferClass::block;
	using BlockBufferClass::max_blocks;
	using BlockBufferClass::tail;
	using BlockBufferClass::size;
	using BlockBufferClass::increase_capacity;
	using BlockBufferClass::append;
	using BlockBufferClass::empty;
	using BlockBufferClass::data;
	using BlockBufferClass::erase;

	using FilterClass::filterWrite;
	
	typedef FilterClass Filter;
	// 0 : normal close ; >0 : current pump bytes
  static uint32_t getBufferTick()
  {
    return BlockBufferClass::s_buffer_tick;
  }

  static uint32_t getBufferTick1()
  {
    return BlockBufferClass::s_buffer_tick1;
  }
  static void resetBufferTick()
  {
    BlockBufferClass::s_buffer_tick = 0;
    BlockBufferClass::s_buffer_tick1 = 0;
  }

	int pump(Sockethelper & so, size_t n = npos)
	{
		if (freespace() < (blocksize() >> 1) 
				&& block() < max_blocks)
			// ignore increase_capacity result.
			increase_capacity(blocksize());

		size_t nrecv = freespace();
		if (nrecv == 0) return -1;
		if (n < nrecv) nrecv = n; // min(n, freespace());

		int ret = so.recv(tail(), nrecv);
		if (ret > 0)
		{
			filterRead(tail(), ret);
			size(size() + ret);
		}
		return ret;
	}

	////////////////////////////////////////////////////////////////////
	// append into buffer only
	void write(char * msg, size_t size)
	{
		if (size == 0) return;

		char *enc = filterWrite(msg, size);
		if (!append(enc, size))
			throw buffer_overflow_all("output buffer overflow [all]");
	}

	void write(Sockethelper & so, SockBuffer & buf)
	{
		write(so, buf.data(), buf.size());
		buf.erase();
	}

	void write(Sockethelper & so, char * msg, size_t size)
	{	// write all / buffered
		if (size == 0) return;
#ifndef WIN32
    struct timeval enter_time; 
    struct timeval middle_time;
    struct timeval leave_time;
    gettimeofday(&enter_time, NULL); 
#endif

		char *enc = filterWrite(msg, size);



		size_t nsent = 0;
		if (empty()) nsent = so.send(enc, size);
#ifndef WIN32
    gettimeofday(&middle_time, NULL);
    int diff_tick = (middle_time.tv_sec - enter_time.tv_sec)*1000000 + (middle_time.tv_usec - enter_time.tv_usec);
    BlockBufferClass::s_buffer_tick += diff_tick;
#endif

		if (!append(enc + nsent, size - nsent))
		{
			// all or part append error .
			if (nsent > 0) throw buffer_overflow("output buffer overflow");
			else throw buffer_overflow_all("output buffer overflow [all]");
    }
#ifndef WIN32
    gettimeofday(&leave_time, NULL);
    int diff_tick1 = (leave_time.tv_sec - enter_time.tv_sec)*1000000 + (leave_time.tv_usec - enter_time.tv_usec);
    BlockBufferClass::s_buffer_tick1 += diff_tick1;
#endif
	}

	int flush(Sockethelper & so, size_t n = npos)
	{
		size_t nflush = size(); if (n < nflush) nflush = n; // std::min(n, size());
		int ret = so.send(data(), nflush);
		erase( 0, ret ); // if call flush in loop, maybe memmove here
		return ret;
	}
};

} // namespace sox

#endif // __SNOX_SOCKBUFFER_H_INCLUDE__
