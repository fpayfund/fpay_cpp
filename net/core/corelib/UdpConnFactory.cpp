#include "UdpConnFactory.h"
#include "core/sox/logger.h"

using namespace core;

UdpServerConnFactory::UdpServerConnFactory() :
	uServer(NULL) {
}

union udpAddr {
	int64_t ipport;
	struct {
		uint32_t ip;
		int port;
	} addrStruct;
};
IConn *UdpServerConnFactory::getConnect(SOCKET so, uint32_t ip, int port,
		ILinkHandler *h, ILinkEvent *eh) {
	udpAddr addr;
	addr.addrStruct.ip = ip;
	addr.addrStruct.port = port;

	//sox::log(Warn, "udp conn ip: ", ip , "udp port: ", port);
	ipport_conn_map_t::iterator it = ipMap.find(addr.ipport);
	if (it != ipMap.end()) {
		return it->second;
	} else {
		UdpConnImp *conn = new UdpConnImp((IConn *)so, ip, port, h, eh);
		conn->setConnId(++cid);
		connects[conn->getConnId()] = conn;
		ipMap[addr.ipport] = conn;
		return conn;
	}
}

void UdpServerConnFactory::eraseConnect(IConn *conn) {
	if (!conn)
		return;
	udpAddr addr;
	addr.addrStruct.ip = conn->getPeerIp();
	addr.addrStruct.port = conn->getPeerPort();
	ipMap.erase(addr.ipport);

	AbstractServerConnFactory::eraseConnect(conn);
}

IConn *UdpServerConnFactory::getServer(const std::string& ip, uint32_t port,
		ILinkHandler *handler, ILinkEvent *eh) {
	UdpServerImp *conn = new UdpServerImp(port, handler, eh);
	conn->setServerConnFactory(this);
	return conn;
}

