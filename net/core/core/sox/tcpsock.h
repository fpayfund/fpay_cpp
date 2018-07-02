
#ifndef __SOX_TCPSOCK_H_INCLUDE__
#define __SOX_TCPSOCK_H_INCLUDE__

#include "snox.h"
#include "hostsox.h"
#include <memory>

namespace sox
{

///////////////////////////////////////////////////////////////////////////////
// Tcpsock

class Tcpsock : public Socket
{
public:
	Tcpsock(SOCKET so) { doAccept(so); }
	Tcpsock(u_long ip, int port, int timo) { doConnect(ip, port, timo); }
	Tcpsock(const std::string & ip, int port, int timo) { doConnect(ip, port, timo); }

	Tcpsock() { }

	void doAccept(SOCKET so) { 
		socket().attach(so);
		socket().setblocking(false);
	}
	void doConnect(const std::string & ip, int port, int timo)
	{ doConnect(aton_addr(ip), port, timo); }
	void doConnect(u_long ip, int port, int timo)
	{
		socket().socket();
		if (timo > 0) socket().setblocking(false);
		if (!socket().connect(ip, port)) select_timeout(timo);
	}

	virtual void select(int remove, int add);
	//virtual ~Tcpsock();

protected:
	virtual void handle(int);

	virtual void onRead()	   = 0;
	virtual void onWrite()	   = 0;
	virtual void onTimeout();
	virtual void onConnected(const std::string & errstr);
};

template < typename ConnectionClass, typename IpClass >
inline ConnectionClass * connect(IpClass ip, int port, int timeout)
{
	log(Warn, "Sox Connect:", ip, " ", port);

	std::auto_ptr<ConnectionClass> at(new ConnectionClass(ip, port, timeout));
	at->select(0, sox::SEL_CONNECTING);
	return at.release();
}

///////////////////////////////////////////////////////////////////////////////
// ServerSocket

enum
{
	SOCKOPT_REUSE	 = 1,
	SOCKOPT_NONBLOCK = 2,
	SOCKOPT_NODELAY  = 4,

	SOCKOPT_DEFAULT  = (SOCKOPT_REUSE | SOCKOPT_NONBLOCK | SOCKOPT_NODELAY),
};

class ServerSocket : public Socket
{
public:
	ServerSocket(SOCKET binded, unsigned int ops = SOCKOPT_DEFAULT);
	ServerSocket(int port, const char * lpszip , unsigned int ops = SOCKOPT_DEFAULT);

	unsigned int get_ops() const { return m_ops; }
	int get_accept_once() const { return m_accept_once; }
	void set_accept_once(int accept_once)
	{
		assert(accept_once > 0);
		m_accept_once = accept_once;
	}
	Hosts * get_hosts_allow() { return m_hosts_allow; }
	void set_hosts_allow(Hosts * hosts_allow) { m_hosts_allow = hosts_allow; }

protected:
	virtual void onAccept(SOCKET so, u_long ip, int port) = 0;
	virtual void handle(int ev);
	virtual void onTimeout();

	Hosts * m_hosts_allow;

	int m_accept_once;
	unsigned int m_ops;
};

template <class Connection >
class Tcpserver : public ServerSocket
{
public:
	Tcpserver(int port, const char * lpszip = NULL, unsigned int ops = SOCKOPT_DEFAULT)
		: ServerSocket(port, lpszip, ops)
	{
	}
protected:
	virtual void onAccept(SOCKET so, u_long ip, int port)
	{
		std::auto_ptr< Connection > conn( new Connection(so) );
		conn->select(0, SEL_READ);
		conn.release();
	}
};

template <class Connection >
class Tcpserver2 : public ServerSocket
{
public:
	Tcpserver2(int port, const char * lpszip = NULL, unsigned int ops = SOCKOPT_DEFAULT)
		: ServerSocket(port, lpszip, ops)
	{
	}
protected:
	virtual void onAccept(SOCKET so, u_long ip, int port)
	{
		std::auto_ptr< Connection > conn( new Connection(so, ip) );
		conn->select(0, SEL_READ);
		conn.release();
	}
};

template < typename ConnectionClass >
class Tcpserver3 : public ServerSocket
{
public:
	Tcpserver3(int port, const char * lpszip)
		: ServerSocket(port, lpszip, SOCKOPT_DEFAULT)
	{
	}
protected:
	virtual void onAccept(SOCKET so, u_long ip, int port)
	{
		std::auto_ptr< ConnectionClass > conn( new ConnectionClass(so, ip, port) );
		conn->select(0, SEL_READ);
		conn.release();
	}
};

} // namespace sox

#endif // __SOX_TCPSOCK_H_INCLUDE__
