
#ifndef __SOX_UDPSOCK_H_INCLUDE__
#define __SOX_UDPSOCK_H_INCLUDE__

#include "snox.h"
#define MAX_UDP_SOCK_SEND_SIZE          (20*1024*1024) //8MB->80MB->20MB
#define MAX_UDP_SOCK_RECV_SIZE          (80*1024*1024) //8MB->80MB->20MB

namespace sox
{

class Udpsock : public Socket
{
public:
	// server
	Udpsock(int localport = 0, const char * localip = NULL)
	{
		udp_create_bind(localport, localip);
		socket().setblocking(false);
	}
	// connect
	Udpsock(const std::string & peerip, int peerport,
		int localport = 0, const char * localip = NULL)
	{
		udp_create_bind(localport, localip);
		udp_connect(peerip, peerport);
//		socket().setblocking(false);
	}

	//only connect long ip
	Udpsock(u_long ip, int port){
		if (!socket().connect(ip, port))
				throw socket_error("udp connect");
//		socket().setblocking(false);
	}
	
	virtual void select(int remove, int add) { Socket::select(remove, add & ~SEL_WRITE); }
	// connected
	void sendto(const void * msg, size_t len) { socket().send(msg, len); }
	// non-connnected
	void sendto(const void * msg, size_t len, const ipaddr_type *to, socklen_t tolen)
	{ socket().sendto(msg, len, to, tolen); }
	int recvfrom(void * buf, size_t len, ipaddr_type *from, socklen_t *fromlen)
	{ return socket().recvfrom(buf, len, from, fromlen); }

protected:
	virtual void onTimeout() { destroy(); }
	virtual void onRead() = 0;

	virtual void handle(int ev)
	{
		switch (ev)
		{
		case SEL_READ : onRead(); return;
		case SEL_TIMEOUT : onTimeout(); return;
		}
		// SEL_WRITE . NEVER
		assert(false);
	}

	void udp_create_bind(int port = 0, const char * lpszip = 0)
	{
		socket().socket(SOCK_DGRAM);
		socket().bind(port, lpszip);
                socket().setsndbuf(MAX_UDP_SOCK_SEND_SIZE);
                socket().setrcvbuf(MAX_UDP_SOCK_RECV_SIZE);
	}

	void udp_connect(const std::string &ip, int port)
	{
		if (!socket().connect(ip, port))
			throw socket_error("udp connect");
	}
};

} // namespace sox

#endif // __SOX_UDPSOCK_H_INCLUDE__
