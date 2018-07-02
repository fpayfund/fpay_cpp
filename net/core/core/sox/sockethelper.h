// Sockethelper.h: interface for the Sockethelper class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SNOX_SOCKETHELPER_H__
#define _SNOX_SOCKETHELPER_H__

#include "exception_errno.h"
#include <string.h>
#include "core/sox/logger.h"
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

namespace sox
{

enum {
	SEL_NONE = 0, SEL_ALL = -1,

	SEL_READ = 1, SEL_WRITE = 2, SEL_RW = 3, // SEL_ERROR = 4,

	// notify only
	SEL_TIMEOUT = 8,

	// setup only, never notify
	// SEL_R_ONESHOT = 32, SEL_W_ONESHOT = 64, SEL_RW_ONESHOT = 96,
	SEL_CONNECTING = 128,
};

class socket_error : public exception_errno
{
public:
	static int getLastError();

	socket_error(const std::string& what_arg)
		: exception_errno(getLastError(), what_arg)
	{}
private:
	friend class Sockethelper;
	socket_error(int e, const std::string& what_arg)
		: exception_errno(e, what_arg)
	{
	}
};

inline bool   valid_addr(u_long ip)             { return ip != INADDR_NONE; }
inline u_long aton_addr(const char * ip)        { return ::inet_addr(ip); }
inline u_long aton_addr(const std::string & ip) { return aton_addr(ip.c_str()); }

// XXX thread ?
inline std::string addr_ntoa(u_long ip)
{ 
	struct in_addr addr;
	memcpy(&addr, &ip, 4);
	return std::string(::inet_ntoa(addr)); 
}

typedef struct sockaddr_in ipaddr_type;

class Sockethelper
{
public:
	virtual ~Sockethelper()  { close(); }
	SOCKET getsocket() const { return m_socket; }
	Sockethelper()           { m_socket = INVALID_SOCKET; }
	void attach(SOCKET so)   { assert(!isValid()); m_socket = so; m_sock_flags.connected = 1; }
	bool isValid() const     { return (getsocket() != INVALID_SOCKET); }
	bool isBlocking() const  { return m_sock_flags.nonblocking == 0; }
	bool isConnected() const { return m_sock_flags.connected == 1; }

	enum { Receives = 1, Sends = 2, Both = 3 };
	void shutdown(int nHow = Sends);
	bool isShutdownSends() const    { return m_sock_flags.shutdown_send == 1; }
	bool isShutdownReceives() const { return m_sock_flags.shutdown_recv == 1; }

	bool checkSendTag() const { return m_sock_flags.send_tag == 1; }
	bool checkRecvTag() const { return m_sock_flags.recv_tag == 1; }
	// clear and return old tag
	bool clearSendTag() { bool b = checkSendTag(); m_sock_flags.send_tag = 0; return b; }
	bool clearRecvTag() { bool b = checkRecvTag(); m_sock_flags.recv_tag = 0; return b; }

	void setRecvTag() { m_sock_flags.recv_tag = 1; }

	// create
	void socket(int type = SOCK_STREAM);

	void setnodelay();
	void setreuse();
	void setblocking(bool blocking);
	void setsndbuf(int size);
	void setrcvbuf(int size);
	int  getsndbuf() const;
	int  getrcvbuf() const;
	int  getavailbytes() const;

	std::string getpeerip(int * port = NULL) const  { return addr_ntoa(getpeer(port)); }
	std::string getlocalip(int * port = NULL) const { return addr_ntoa(getlocal(port)); }
	u_long getpeer(int * port = NULL) const;
	u_long getlocal(int * port = NULL) const;

	// >0 : bytes recv
	// 0  : peer close normal
	// <0 : error (reserve)
	//    : throw socket-error
	int recv(void * lpBuf, size_t nBufLen, int flag=0);
	// >=0 : bytes send;
	// <0  : error (reserve)
	//     : throw socket_error
	int send(const void * lpBuf, size_t nBufLen, int flag=0);
	// blocking mode; nonblock may work
	void sendall(const char * buf, size_t len);

	// use poll or select(WIN32) to wait
	int waitevent(int event, int timeout);

	// >0 : bytes sendto
	// 0  : isOk or isIgnoreError
	// <0 : error (reserve)
	//    : throw socket_error
	int sendto(const void * msg, size_t len, const ipaddr_type *to, socklen_t tolen);
	int sendto(const void * msg, size_t len, u_long ip, int port);
	int sendto(const void * msg, size_t len, const std::string &ip, int port)
	{ return sendto(msg, len, aton_addr(ip), port); }
	// >0 : bytes recvfrom
	// 0  : isOk or isIgnoreError
	// <0 : error (reserve)
	//    : throw socket_error
	int recvfrom(void * buf, size_t len, ipaddr_type *from, socklen_t *fromlen);

	// true  : connect completed
	// false : nonblocking-connect inprocess
	//       : throw socket_error
	bool connect(const std::string & ip, int port) { return connect(aton_addr(ip), port); }
	bool connect(u_long ip, int port);
	std::string complete_nonblocking_connect();

	// >=0 : accepted socket
	// <0  : some error can ignore. for accept more than once
	//     : throe socket_error
	SOCKET accept(u_long * addr = NULL, int * port = NULL);
	SOCKET accept(std::string * ip, int * port = NULL);

	void bind(int port, const std::string & ip);
	void bind(int port, const char * lpszip);
	void bind(int port, u_long addr = INADDR_ANY);
	void listen();

	// init
	static void initialize();
	static void finalize();

	static int soclose(SOCKET so);

	static SOCKET try_bind(const char* ip, unsigned short & port);
  static void timer_reset()
  {
    log(Info, "OutputBuffer s_sock_tick:%u", s_sock_tick);
    s_sock_tick = 0;
  }

	SOCKET detach();

protected:
  static  uint32_t s_sock_tick;

protected:
	bool getsockopt(int level, int optname, void *optval, socklen_t *optlen) const
	{ return (SOCKET_ERROR != ::getsockopt(getsocket(), level, optname, (char*)optval, optlen)); }
	bool setsockopt(int level, int optname, const void *optval, socklen_t optlen)
	{ return (SOCKET_ERROR != ::setsockopt(getsocket(), level, optname, (const char *)optval, optlen)); }

	bool IOCtl(long cmd, u_long * arg) const;
	int how_shutdown(int nHow);
	bool isIgnoreConnect(int en) const;
	bool isIgnoreError(int en) const;
	bool isIgnoreAcceptError(int en) const;

	struct SockFlags {
		// used by Socket
		unsigned int selevent : 8;
		unsigned int selected : 1;
		// inner
		unsigned int connected     : 1;
		unsigned int nonblocking   : 1;
		unsigned int tcpserver     : 1;
		unsigned int shutdown_send : 1;
		unsigned int shutdown_recv : 1;

		unsigned int send_tag : 1;
		unsigned int recv_tag : 1;

		SockFlags()  { reset(); }
		void reset() { *((unsigned int*)this) = 0; } /* XXX clear all */
	} m_sock_flags;

	friend class Socket;
	void close(); // careful if it's selected. hide now
	
private:
	SOCKET m_socket;
};

#if defined(WIN32) && !defined(__CYGWIN32__)
#include "sockethelper_win.inc"
#else
#include "sockethelper_unx.inc"
#endif

inline int Sockethelper::recv(void * lpBuf, size_t nBufLen, int flag)
{
	m_sock_flags.recv_tag = 1;
	int ret = ::recv(getsocket(), (char*)lpBuf, SOX_INT_CAST(nBufLen), flag);
	if (SOCKET_ERROR == ret)
	{
		// XXX howto ignore EAGAIN and EINTR
		throw socket_error("recv");
	}
	return ret;
}

inline int Sockethelper::send(const void * lpBuf, size_t nBufLen, int flag)
{
	m_sock_flags.send_tag = 1;

	int ret = ::send(getsocket(), (const char*)lpBuf, SOX_INT_CAST(nBufLen), flag);

	if (SOCKET_ERROR == ret)
	{
		int en = socket_error::getLastError();
		if (isIgnoreError(en)) return 0;
		throw socket_error(en, "send");
	}
	return ret;
}

inline int Sockethelper::sendto(const void * msg, size_t len, u_long ip, int port)
{
	ipaddr_type sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ip;
	sa.sin_port = htons((u_short)port);

	return sendto(msg, len, &sa, sizeof(sa));
}

inline int Sockethelper::sendto(const void * msg, size_t len, const ipaddr_type *to, socklen_t tolen)
{
	m_sock_flags.send_tag = 1;
	int ret = ::sendto(getsocket(), (const char *)msg, SOX_INT_CAST(len), 0, (struct sockaddr*)to, tolen);

	if (SOCKET_ERROR == ret)
	{
		int en = socket_error::getLastError();
		if (isIgnoreError(en)) return 0;
		throw socket_error(en, "sendto");
	}
	return ret;
}

inline int Sockethelper::recvfrom(void * buf, size_t len, ipaddr_type *from, socklen_t *fromlen)
{
	m_sock_flags.recv_tag = 1;
	int ret = ::recvfrom(getsocket(), (char *)buf, SOX_INT_CAST(len), 0, (struct sockaddr*)from, fromlen);

	if (SOCKET_ERROR == ret)
	{
		int en = socket_error::getLastError();
		if (isIgnoreError(en)) return 0; // igore again ant interrupt
		throw socket_error(en, "recvfrom");
	}
	return ret;
}

inline void Sockethelper::socket(int type)
{
	m_socket = ::socket(AF_INET, type, 0);

	if (SOCKET_ERROR == m_socket)
		throw socket_error("create socket");
	
#ifndef WIN32
	else
		fcntl(m_socket, F_SETFD, fcntl(m_socket, F_GETFD) | FD_CLOEXEC);
#endif
}

inline SOCKET Sockethelper::accept(std::string * ip, int * port)
{
	u_long addr;
	int pot;
	SOCKET newso = accept(&addr, &pot);
	if (ip) *ip = addr_ntoa(addr);
	if (port) *port = pot;
	return newso;
}

inline SOCKET Sockethelper::accept(u_long * addr, int * port)
{
	ipaddr_type sa;
	socklen_t len = sizeof(sa);

	SOCKET ret = ::accept(getsocket(), (struct sockaddr*)&sa, &len);
	if (SOCKET_ERROR == ret)
	{
		int en = socket_error::getLastError();
		if (isIgnoreAcceptError(en))
			return SOCKET_ERROR;
		throw socket_error(en, "accept");
	}

	if (addr) *addr = sa.sin_addr.s_addr;
	if (port) *port = ntohs(sa.sin_port);
	return ret;
}

} // namespace sox

#endif // _SNOX_SOCKETHELPER_H__

