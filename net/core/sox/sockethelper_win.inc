
// 没有考虑 WSAEINTR, 根本不准备调用 WSACancelBlockingCall

inline int socket_error::getLastError()
{
	return WSAGetLastError();
}

inline bool Sockethelper::isIgnoreAcceptError(int en) const
{
	return WSAEWOULDBLOCK == en;
}

inline bool Sockethelper::isIgnoreError(int en) const
{
	return WSAEWOULDBLOCK == en;
}

inline bool Sockethelper::isIgnoreConnect(int en) const
{
	return WSAEWOULDBLOCK == en;
}

inline void Sockethelper::setblocking(bool blocking)
{
	// note that this will not work on some versions of Windows for Workgroups. Tough. -- jfc

	// this will not work if you are using WSAAsyncSelect or WSAEventSelect.
	// -- perhaps I should throw an exception

	unsigned long op = (blocking ? 0 : 1);
	if (SOCKET_ERROR == ::ioctlsocket(getsocket(), FIONBIO, &op))
		throw socket_error("setblocking FIONBIO");

	m_sock_flags.nonblocking = (blocking ? 0 : 1);
}

inline bool Sockethelper::IOCtl(long cmd, u_long * arg) const
{
	return (SOCKET_ERROR != ::ioctlsocket(getsocket(), cmd, arg));
}

inline void Sockethelper::initialize()
{
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(1, 1);
	int nResult = WSAStartup(wVersionRequested, &wsaData);
	if (nResult != 0)
		throw socket_error(nResult, "WSAStartup");

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		// the call to WSAStartup caused Winsock to initialize, WSACleanup will be
		// called automatically on program shutdown.
		// call getaddrinfo() eg... functions  after Winsock has shutdown, which can lead to crashes.
		
		//WSACleanup();
		throw socket_error(0, "wsasocket unknown version");
	}
}

inline void Sockethelper::finalize()
{
	 // the call to WSAStartup caused Winsock to initialize, WSACleanup will be
	 // called automatically on program shutdown.
	 // call getaddrinfo() eg... functions  after Winsock has shutdown, which can lead to crashes.
	 
	 //WSACleanup();
}

inline int Sockethelper::how_shutdown(int nHow)
{
	switch (nHow)
	{
	case Receives :
		return SD_RECEIVE;
	case Sends :
		return SD_SEND;
	default : //case Both :
		return SD_BOTH;
	}
}

inline int Sockethelper::soclose(SOCKET so)
{
	return ::closesocket(so);
}

#define SOX_INT_CAST(x) int(x)


