
inline int socket_error::getLastError()
{
	return errno;
}

inline bool Sockethelper::isIgnoreAcceptError(int en) const
{
	return en == EWOULDBLOCK || en == EINTR || en == ECONNABORTED || en == EMFILE;
}

inline bool Sockethelper::isIgnoreError(int en) const
{
	return en == EAGAIN || en == EINTR || EINPROGRESS == en;
}

inline bool Sockethelper::isIgnoreConnect(int en) const
{
	return EWOULDBLOCK == en || EINPROGRESS == en;
}

inline void Sockethelper::setblocking(bool blocking)
{
	int fflags = ::fcntl(getsocket(), F_GETFL);
	if (SOCKET_ERROR == fflags)
		throw socket_error("setblocking F_GETFL");

	if (blocking) fflags &= ~O_NONBLOCK;
	else          fflags |= O_NONBLOCK;

	if (SOCKET_ERROR == ::fcntl(getsocket(), F_SETFL, fflags))
		throw socket_error("setblocking F_SETFL");

	m_sock_flags.nonblocking = (blocking ? 0 : 1);
}

inline bool Sockethelper::IOCtl(long cmd, u_long * arg) const
{
	return (SOCKET_ERROR != ::ioctl(getsocket(), cmd, arg));
}

inline void Sockethelper::initialize()
{
	// nothing
}

inline void Sockethelper::finalize()
{
	// nothing
}

inline int Sockethelper::how_shutdown(int nHow)
{
	switch (nHow)
	{
	case Receives :
		return SHUT_RD;
	case Sends :
		return SHUT_WR;
	default: // Both :
		return SHUT_RDWR;
	}
}

inline int Sockethelper::soclose(SOCKET so)
{
	return ::close(so);
}

#define SOX_INT_CAST(x) x
