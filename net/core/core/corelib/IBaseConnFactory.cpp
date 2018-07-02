#include "common/core/ilink.h"
#include "core/sox/logger.h"
#include "common/core/request.h"

using namespace core;

IConnManager::~IConnManager() {

}


IConn *IConnManager::createServerConn(SOCKET so, uint32_t ip, int port, ILinkHandler *h, ILinkEvent *eH){
	IConn *conn = serverCreator->creatConnect(so, ip, port, h, eH, this);
	return conn;
}

IConn *IConnManager::createClientConn(const std::string& ip, uint32_t port, ILinkHandler *iH, ILinkEvent *eH){
	IConn *conn = clientCreator->createConnect(ip, port, iH, eH, this);
	return conn;
}
