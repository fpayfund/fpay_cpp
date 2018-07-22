
#ifndef __SOX_EXCEPTION_ERRNO_H_INCLUDE__
#define __SOX_EXCEPTION_ERRNO_H_INCLUDE__

#include "common/socketinc.h"

#include <stdexcept>
#include <string>

#include "toString.h"

namespace sox
{

class exception_errno : public std::exception
{
public:
	exception_errno(const std::string& str)        { init(str); }
	exception_errno(int e, const std::string& str) { init(e, str); }

	const std::string & what_str() const throw();
	int what_errno() const		   throw() { return m_errno; }
	virtual const char * what () const throw() { return what_str().c_str (); }

	~exception_errno() throw() { }
private:
	exception_errno() : exception () { }
	void init(int e, const std::string & str)
	{
		m_errno = e;
		m_what = str;
		m_bstrerror = false;
	}
	void init(const std::string & str) { init(lasterrno(), str); }
	int lasterrno()
	{
#if defined(WIN32)
		return GetLastError();
#else
		return errno;
#endif
	}
	int m_errno;
	mutable std::string m_what;
	mutable bool m_bstrerror;
};

} // namespace sox

#endif // __SOX_EXCEPTION_ERRNO_H_INCLUDE__
