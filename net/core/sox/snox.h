
#ifndef _SNOX_H_INCLUDE_
#define _SNOX_H_INCLUDE_

#include "selector.h"
#include <time.h>

namespace sox
{

class Handler;

struct env
{
	static Selector * selector()	     { return m_sel; }
	static void selector(Selector * sel) { m_sel = sel; }
	static void finalize();

	static int SocketErrorLogLevel;
	static time_t now;
	static std::string strTime;
	static unsigned int  haomiao;
private:
	static Selector * m_sel;
	friend class Selector;
	friend class Handler;
	static Handler * s_CurrentHandler;
};

class Handler
{
public:
	// handle event. notify one by one
	virtual void handle(int) = 0;
	// selector ���ǵ���������ݻ�����. ��Ӧ����Ҫ����.
	virtual void destroy() { delete this; } 
	virtual std::ostream & trace(std::ostream & os) const;
	Handler() { }

	enum { INFTIMO = -1, MAXTIMO = (size_t(-1) >> 1) };
	void select_timeout(int timeout = INFTIMO)
	{ 
		Selector * s = env::selector();
		if( s != NULL ) {
			env::selector()->countdown().select_timeout(this, timeout); 
		}
	}

	// ��ǰ�����ǲ������ڽ����¼�, env::s_CurrentHandler �� selector::notify ����
	bool isCurrentHandler() const   { return this == env::s_CurrentHandler; }
	
	std::string toString() const;
protected:
	virtual ~Handler();
	friend class Countdown;
	// manage by Countdown
	Countdown::iterator m_iter_timo;
	// non-copyable
	Handler(const Handler &){}
	void operator = (const Handler &);
};

inline std::ostream & operator << (std::ostream & os, const Handler & h)
{
	return h.trace(os);
}

class Socket : public Handler
{
public:
	//virtual void close();
	virtual void select(int remove, int add);
	virtual void remove();
	virtual std::ostream & trace(std::ostream & os) const;

	Sockethelper & socket()         { return m_socket; }
protected:
	virtual ~Socket();
	Sockethelper m_socket;
};

} // namespace sox

#endif // _SNOX_H_INCLUDE_
