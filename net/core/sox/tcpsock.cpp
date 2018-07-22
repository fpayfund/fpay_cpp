#include "tcpsock.h"

namespace sox {

///////////////////////////////////////////////////////////////////////////////
// Tcpsock

/*
 Tcpsock::~Tcpsock()
 {
 //log(DEBUG, "Tcpsock dtor ", *this);
 }
 */

void Tcpsock::select(int remove, int add) {
	if (add & SEL_CONNECTING) {
		if (socket().isConnected()) {
			onConnected("connect ok immediately");
			return;
		}
		add = SEL_RW;
	}
	Socket::select(remove, add);
}

void Tcpsock::handle(int ev) {
	// handle event. notify one by one
	if (socket().isConnected()) {
		switch (ev) {
		case SEL_READ :
			onRead();
			return;
		case SEL_WRITE :
			onWrite();
			return;
		case SEL_TIMEOUT :
			onTimeout();
			return;
		}
	} else {
		switch (ev) {
		case SEL_TIMEOUT :
			onConnected("connect timeout");
			return;
		case SEL_READ :
		case SEL_WRITE :
			onConnected(socket().complete_nonblocking_connect());
			return;
		}
	}
	assert(false);
}

void Tcpsock::onTimeout() {
	log(Info, "time out: ");
	destroy();
}

void Tcpsock::onConnected(const std::string & errstr) {
	// nonblock connect, must handle this
	log(Error, "%s, %s", errstr.data(), toString().data());
	assert(false);
}

ServerSocket::ServerSocket(int port, const char * lpszip, unsigned int ops) {
	m_hosts_allow = NULL;
	m_accept_once = 1;

	socket().socket();

	if (ops & SOCKOPT_REUSE)
		socket().setreuse();
	if (ops & SOCKOPT_NONBLOCK)
		socket().setblocking(false);
	if (ops & SOCKOPT_NODELAY)
		socket().setnodelay();

	m_ops = ops;
	socket().bind(port, lpszip);
	socket().listen();
}

ServerSocket::ServerSocket(SOCKET binded, unsigned int ops){
	m_hosts_allow = NULL;
	m_accept_once = 1;
	socket().attach(binded);

	if (ops & SOCKOPT_REUSE)
		socket().setreuse();
	if (ops & SOCKOPT_NONBLOCK)
		socket().setblocking(false);
	if (ops & SOCKOPT_NODELAY)
		socket().setnodelay();

	m_ops = ops;
	socket().listen();
}
void ServerSocket::handle(int ev) {
	try { // XXX 忽略所有 accept 错误，只记录

		switch (ev)
		{
			case SEL_READ: {
				// 可以忽略的错误直接返回 SOCKET_ERROR
				u_long ip; int port; SOCKET s;
				//int max = m_accept_once;
				//xxx modify to accept multi
				while ((s = socket().accept(&ip, &port)) != SOCKET_ERROR)
				{
					if (NULL == m_hosts_allow || m_hosts_allow->find(ip))
					{
						onAccept(s, ip, port);
						continue;
					}
					log(Warn, "[ServerSocket::handle] access deny from=%s:%d", addr_ntoa(ip).data(), port);
					Sockethelper::soclose(s);
				} // while
				
			} // case SEL_READ
			break;
			case SEL_TIMEOUT: onTimeout(); break;
			default: assert(false); break;
		}

	} catch (std::exception &ex) {
		log(Error, "ServerSocket::handle %s", ex.what());
	}
}

void ServerSocket::onTimeout() {
	assert(false); // must
}

} // namespace sox
