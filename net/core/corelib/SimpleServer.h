#ifndef SIMPLESERVER_H_
#define SIMPLESERVER_H_
#include "WrapServerStart.h"
#include "common/core/iserver.h"
#include "core/sox/tcpsock.h"
#include "core/sox/selsel.h"
#include "MAbstractServer.h"
namespace core {


class MSimpleServer : public MAbstractServer, public sox::ServerSocket {
	std::string ip;

	uint16_t port;

public:
	MSimpleServer(const char * ip, int port) : sox::ServerSocket(port, ip) {
		if(ip){
			this->ip = ip;
		}
		this->port = (int16_t)port;
	}

	virtual void startSV() {
		this->select(0, sox::SEL_READ);
	}

	virtual void hangUp(){}

	virtual void wakeUp(){}


	virtual std::string getIp(){
		return ip;
	}

	virtual std::vector<uint16_t> getPorts(){
		std::vector<uint16_t> ps;
		ps.push_back(port);
		return ps;
	}
protected:
	virtual void onAccept(SOCKET so, u_long ip, int port) {
		connManager->createServerConn(so, ip, port, handler, getLinkEvent());
	}
};

}
#endif /*SIMPLESERVER_H_*/
