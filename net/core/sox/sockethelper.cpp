
#include "sockethelper.h"
#include "logger.h"

namespace sox
{
uint32_t Sockethelper::s_sock_tick = 0;

SOCKET Sockethelper::detach()
{
	SOCKET so = m_socket;
	m_socket = INVALID_SOCKET;
	m_sock_flags.reset();
	return so;
}

void Sockethelper::shutdown(int nHow)
{
	if (nHow & Receives)
	{
		if (m_sock_flags.selevent & SEL_READ)
			throw socket_error(0, "shutdown recv, but SEL_READ setup");
		m_sock_flags.shutdown_recv = 1;
	}
	if (nHow & Sends)
	{
		if (m_sock_flags.selevent & SEL_WRITE)
			throw socket_error(0, "shutdown send, but SEL_WRITE setup");
		m_sock_flags.shutdown_send = 1;
	}
	//int ret = 
	::shutdown(getsocket(), how_shutdown(nHow));
	// XXX if (SOCKET_ERROR == ret) {}
}

void Sockethelper::close()
{
	if (isValid())
	{
		//int ret = 
		soclose(m_socket);
		// XXX if (SOCKET_ERROR == ret) {}
		m_sock_flags.reset();
		m_socket = INVALID_SOCKET;
	}
}

void Sockethelper::setnodelay()
{
	int op = 1;
	if (!setsockopt(IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op)))
		throw socket_error("setnodelay");
}

void Sockethelper::setreuse()
{
	int op = 1;
	if (!setsockopt(SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)))
		throw socket_error("setreuse");
}

int  Sockethelper::getsndbuf() const
{
	int size = 0;
	socklen_t len = sizeof(size);
	if (!getsockopt(SOL_SOCKET, SO_SNDBUF, &size, &len))
		throw socket_error("getsndbuf");
	return size;
}

int  Sockethelper::getrcvbuf() const
{
	int size = 0;
	socklen_t len = sizeof(size);
	if (!getsockopt(SOL_SOCKET, SO_RCVBUF, &size, &len))
		throw socket_error("getrcvbuf");
	return size;
}

void Sockethelper::setsndbuf(int size)
{
	if (!setsockopt(SOL_SOCKET, SO_SNDBUF, &size, sizeof(size)))
		throw socket_error("setsndbuf");
}

void Sockethelper::setrcvbuf(int size)
{
	if (!setsockopt(SOL_SOCKET, SO_RCVBUF, &size, sizeof(size)))
		throw socket_error("setrcvbuf");
}

int Sockethelper::getavailbytes() const
{
	if (m_sock_flags.tcpserver)
		return -1; // is tcp-server

	u_long ret;
	if (!IOCtl(FIONREAD, &ret))
		throw socket_error("getavailbytes");
	return ret;
}

void Sockethelper::sendall(const char * buf, size_t len)
{
	while (len > 0)
	{
		int n = send(buf, len);
		if (n > 0)
		{
			len -= n;
			buf += n;
		}
		// (n==0) // try send first. throw if nonblock and send failed
		else if (!isBlocking())
			throw socket_error(0, "send_all but nonblock");
		// blocking and n==0 is ok. restart interrupt default
	}
}

void Sockethelper::listen()
{
	if (SOCKET_ERROR == ::listen(getsocket(), SOMAXCONN))
		throw socket_error("listen");
	m_sock_flags.tcpserver = 1;
}

void Sockethelper::bind(int port, const std::string & ip)
{
	u_long addr = INADDR_ANY;
	if (!ip.empty())
	{
		addr = ::inet_addr(ip.c_str());
		if (addr == INADDR_NONE)
			throw socket_error(0, tostring("bind: invalid ip=", ip));
	}
	bind(port, addr);
}

void Sockethelper::bind(int port, u_long addr)
{
	ipaddr_type sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = addr;
	sa.sin_port = htons((u_short)port);

	if (SOCKET_ERROR == ::bind(getsocket(), (struct sockaddr*)&sa, sizeof(sa)))
		throw socket_error(tostring("bind at ", port));
}

void Sockethelper::bind(int port, const char * lpszip)
{
	u_long addr = INADDR_ANY;
	if (lpszip && strlen(lpszip) > 0)
	{
		addr = ::inet_addr(lpszip);
		if (addr == INADDR_NONE)
			throw socket_error(0, tostring("bind: invalid ip=", lpszip));
	}
	bind(port, addr);
}

u_long Sockethelper::getpeer(int *port) const
{
	ipaddr_type sa;
	memset(&sa, 0, sizeof(sa));
	socklen_t len = sizeof(sa);

	if (SOCKET_ERROR == ::getpeername(getsocket(), (struct sockaddr *)&sa, &len))
		throw socket_error("getpeer");

	if (port) *port = ntohs(sa.sin_port);
	return sa.sin_addr.s_addr;
}

u_long Sockethelper::getlocal(int *port) const
{
	ipaddr_type sa;
	memset(&sa, 0, sizeof(sa));
	socklen_t len = sizeof(sa);

	if (SOCKET_ERROR == ::getsockname(getsocket(), (struct sockaddr*)&sa, &len))
		throw socket_error("getlocal");

	if (port) *port = ntohs(sa.sin_port);
	return sa.sin_addr.s_addr;
}

bool Sockethelper::connect(u_long ip, int port)
{
	ipaddr_type sa;
	memset(&sa, 0, sizeof(sa));

	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ip;
	sa.sin_port = htons((u_short)port);

	if (SOCKET_ERROR == ::connect(getsocket(), (struct sockaddr*)&sa, sizeof(sa)))
	{
		int en = socket_error::getLastError();
		log(Info, "error socket port:%d, error no is:%d", port, en);
		if (isIgnoreConnect(en))
			return false;
		throw socket_error(en, "connect");
	}
	// XXX check dereism [freebsd]
	{
	u_long localip; int localport;
	localip = getlocal(&localport);
	if (localip == ip && localport == port)
		throw socket_error(0, "dereism connection found");
	}
	m_sock_flags.connected = 1;
	return true;
}

std::string Sockethelper::complete_nonblocking_connect()
{
	int e = 0;
	socklen_t l = sizeof(e);

	if (!getsockopt(SOL_SOCKET, SO_ERROR, &e, &l))
		e = socket_error::getLastError();
	if (e)
	{
		socket_error err(e, "connect noneblock");
		return err.what_str();
	}
	m_sock_flags.connected = 1;
	return std::string("connect ok");
}

#if defined(WIN32)
int Sockethelper::waitevent(int event, int timeout)
{
	if (!isValid())
		throw socket_error(0, "poll invalid socket");

	fd_set rs;
	fd_set ws;
	FD_ZERO(&rs);
	FD_ZERO(&ws);
	if (event & SEL_READ) FD_SET(getsocket(), &rs);
	if (event & SEL_WRITE) FD_SET(getsocket(), &ws);
	
	int ret;
	if (timeout >= 0)
	{
		struct timeval tv;
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = 1000 * (timeout % 1000);
		ret = ::select(int(getsocket()) + 1, &rs, &ws, NULL, &tv);
	}
	else
		ret = ::select(int(getsocket()) + 1, &rs, &ws, NULL, NULL);

	if (SOCKET_ERROR == ret) throw socket_error("select");
	if (0 == ret) return 0; // timeout

	// ok, return ready event
	ret = 0;
	if (FD_ISSET(getsocket(), &rs)) ret |= SEL_READ;
	if (FD_ISSET(getsocket(), &ws)) ret |= SEL_WRITE;
	return ret;
}

#else

int Sockethelper::waitevent(int event, int timeout)
{
	if (!isValid())
		throw socket_error(0, "poll invalid socket");

	struct pollfd fds[1];
	fds[0].fd = getsocket();
	fds[0].events = 0;

	if (event & SEL_READ) fds[0].events |= POLLIN;
	if (event & SEL_WRITE) fds[0].events |= POLLOUT;

	int ret = ::poll(fds, 1, timeout);
	if (SOCKET_ERROR == ret)
	{
		if (errno == EINTR)
			return -1;
		throw socket_error("poll");
	}
	if (0 == ret) return 0; // timeout

	if (fds[0].revents & POLLERR) throw socket_error(0, "POLLERR");
	if (fds[0].revents & POLLNVAL) throw socket_error(0, "POLLNVAL");

	// ok, return ready event
	ret = 0;
	if (fds[0].revents & POLLIN) ret |= SEL_READ;
	if (fds[0].revents & POLLOUT) ret |= SEL_WRITE;
	return ret;
}
#endif

SOCKET Sockethelper::try_bind( const char* ip, unsigned short & port )
{
	for ( unsigned short i = 0; i < 100; ++i, ++port ) {
		try {
			sox::Sockethelper so;
			so.socket();
			so.setreuse();
			so.setblocking(false);
			so.setnodelay();
			so.bind(port, ip);
			return so.detach();
		} catch( std::exception & e ) {
			log( Warn, "bind socket failed: %s.", e.what() );
		}
	}

	return INVALID_SOCKET;
}

}  // namespace sox
