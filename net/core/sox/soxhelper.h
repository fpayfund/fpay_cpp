
#ifndef __SOX_UTIL_INC_HELPER_H_INCLUDE__
#define __SOX_UTIL_INC_HELPER_H_INCLUDE__

#include "common/soxhelper.h"

namespace sox
{

std::string hex2bin(const void * _src, size_t len, bool strict = false);
inline std::string hex2bin(const std::string & src, bool strict = false)
{ return hex2bin(src.data(), src.size(), strict); }

extern int Base64Encode(void *_dst, const void *_src, size_t len);
extern int Base64Decode(void *_dst, const void *_src, size_t len);

///////////////////////////////////////////////////////////////////////////
inline std::string base64decode(const std::string & s)
{
	std::string dst;
	dst.resize(s.size()); // get memory. reserve may not work
	// truncate to actual size   XXX const_cast. 
	dst.resize(Base64Decode(const_cast<char *>(dst.data()), s.data(), s.size()));
	return dst;
}

inline std::string base64encode(const std::string & s)
{
	std::string dst;
	dst.resize((s.size() / 3 + 1) * 4); // get memory. reserve may not work
	dst.resize(Base64Encode(const_cast<char *>(dst.data()), s.data(), s.size()));
	return dst;
}

/*
// XXX for short single-copy str. store. c style. if multi-copy, use string 
struct cstr
{
	cstr()  { m_data = NULL; }
	~cstr() { delete [] m_data; }

	cstr(const cstr & x)             { m_data = x.release(); }
	cstr & operator=(const cstr & x)
	{
		if (&x != this)
		{
			delete[] m_data;
			m_data = x.release();
		}
		return *this;
	}

	cstr(const char * s)        { m_data = NULL; reset(s, strlen(s)); }
	cstr(const std::string & s) { m_data = NULL; reset(s.data(), s.size()); }
	cstr & operator=(const std::string & s) { reset(s.data(), s.size()); return *this; }
	cstr & operator=(const char * s) { reset(s, strlen(s)); return *this; }

	operator const char * () const { return data(); }
	const char * data() const { return m_data?m_data:""; }
	size_t size() const { return m_data?strlen(m_data):0; }
	bool empty() const { return size() == 0; }

private:
	void reset(const char * s, size_t size)
	{
		delete [] m_data;
		if (size > 0) {
			m_data = new char[size + 1];
			memcpy(m_data, s, size);
			m_data[size] = 0;
		}
		else m_data = NULL;
	}
	char * release() const { char * data = m_data; m_data = NULL; return data; }
	mutable char * m_data;
};

*/
} // namespace sox

#endif // __SOX_UTIL_INC_HELPER_H_INCLUDE__
