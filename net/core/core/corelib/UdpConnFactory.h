#ifndef UDPCONNFACTORY_H_
#define UDPCONNFACTORY_H_

#include "AbstractServerConnFactory.h"
#include "UdpConnImp.h"

#include <map>
namespace core
{
class UdpServerConnFactory : public AbstractServerConnFactory
{
	typedef std::map<int64_t, IConn * > ipport_conn_map_t;
	ipport_conn_map_t ipMap;
	UdpServerImp *uServer;
public:
	UdpServerConnFactory();
	
	virtual IConn *getConnect(SOCKET so, uint32_t ip, int port, ILinkHandler *h, ILinkEvent *eh);

	virtual void eraseConnect(IConn *conn);

	IConn *getServer(const std::string& ip, uint32_t port,
		ILinkHandler *handler, ILinkEvent *eh);
};


}

#endif /*UDPCONNFACTORY_H_*/
