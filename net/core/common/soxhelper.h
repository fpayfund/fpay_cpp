
#ifndef __SOX_UTIL_HELPER_H_INCLUDE__
#define __SOX_UTIL_HELPER_H_INCLUDE__

#include <string>
#include <string.h>
#include <algorithm>
#include <memory>
#include <time.h>

#include "common/varstr.h"

namespace sox
{

/** simple check only */
inline unsigned xatol(const char * s, size_t len, const char ** end = NULL)
{
	long n = 0;
	size_t i = 0;
	for (; i<len; ++i)
	{
		int t = s[i] - '0';
		if (t>9 || t<0) break;
		n = n * 10 + t;
	}
	if (end) *end = (i<len) ? (s + i) : NULL;
	return n;
}

/**  uint2string */
inline std::string ultoa10(size_t n)
{
	char abuf[16]; // XXX
	char *p = abuf + 15;
	*p-- = 0;
	do
	{
		*p-- = char('0' + (n % 10));
	} while (n /= 10);
	return std::string(p + 1, abuf + 14 - p);
}

inline int strcasecmp(const char * s1, const char * s2)
{
#ifdef WIN32
	return ::_stricmp(s1, s2);
#else
	return ::strcasecmp(s1, s2);
#endif
}

inline int strncasecmp(const char * s1, const char * s2, size_t n)
{
#ifdef WIN32
	return ::_strnicmp(s1, s2, n);
#else
	return ::strncasecmp(s1, s2, n); // BSD ?
#endif
}

inline std::string bin2hex(const void * _src, size_t len)
{
	const unsigned char * Hex = (const unsigned char *)"0123456789abcdef"; 
	const unsigned char * src = (const unsigned char *)_src; 

	std::string hex;
	hex.resize(len << 1);

	for (size_t i = 0; i < len; i++)
	{
		hex[i*2] = Hex[src[i] >> 4 ];
		hex[i*2+1] = Hex[src[i] & 0xF];
	}

	return hex;
}

inline std::string bin2hex(const std::string & src)
{
	return bin2hex(src.data(), src.size());
}

/** * return len include the char . 0 notfound */
inline size_t find(const char * s, size_t l, char c = '\n')
{
#ifdef __GLIBC__
	const char * f = (const char *)memchr(s, c, l);
	return (f ? (f-s+1) : 0);
#else
	size_t e = 0;
	for (; e < l && s[e] != c; ++e) {}
	return (e < l ? (e + 1) : 0);
#endif
}

inline bool endsWith(const std::string & big, const std::string & little)
{
	if (big.size() < little.size()) return false;
	return std::equal(little.rbegin(), little.rend(), big.rbegin());
}

inline bool startsWith(const std::string & big, const std::string & little)
{
	if (big.size() < little.size()) return false;
	return std::equal(little.begin(), little.end(), big.begin());
}

/**  find \n\n . copy from [squid] */
extern size_t headersEnd(const char *mime, size_t l);

inline void localtimex(const time_t & t, struct tm & m)
{
#ifdef WIN32
	//m = *::localtime(&t); //可能时间越界返回NULL,而造成崩溃。
	struct tm *pTmp = ::localtime(&t);
	if(pTmp)
	{
		m = *pTmp;
	}
	else
	{
		//set one time
		m.tm_sec  = 0;    /* seconds after the minute - [0,59] */
		m.tm_min  = 0;    /* minutes after the hour - [0,59] */
		m.tm_hour = 0;    /* hours since midnight - [0,23] */
		m.tm_mday = 0;    /* day of the month - [1,31] */
		m.tm_mon  = 0;    /* months since January - [0,11] */
		m.tm_year = 111;  /* years since 1900 */
		m.tm_wday = 0;    /* days since Sunday - [0,6] */
		m.tm_yday = 0;    /* days since January 1 - [0,365] */
		m.tm_isdst= 0;   /* daylight savings time flag */
	}
#else
	::localtime_r(&t, &m);
#endif
}

inline std::string FormatTime(const struct tm & m)
{
	std::string s;
	s = s + sox::ultoa10(m.tm_year + 1900)
		+ ((m.tm_mon < 9) ? "-0" : "-") + sox::ultoa10(m.tm_mon + 1)
		+ ((m.tm_mday < 10) ? "-0" : "-") + sox::ultoa10(m.tm_mday)
		+ ((m.tm_hour < 10) ? " 0" : " ") + sox::ultoa10(m.tm_hour)
		+ ((m.tm_min < 10) ? ":0" : ":") + sox::ultoa10(m.tm_min)
		+ ((m.tm_sec < 10) ? ":0" : ":") + sox::ultoa10(m.tm_sec)
		;

	return s;
}

inline std::string FormatTime(const time_t &t)
{
	struct tm m;
	localtimex(t, m);

	return FormatTime(m);
}

} // namespace sox

#endif // __SOX_UTIL_HELPER_H_INCLUDE__
