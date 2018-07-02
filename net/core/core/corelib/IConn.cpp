#include "common/core/ilink.h"
#include "core/sox/logger.h"
#include "core/sox/sockethelper.h"
using namespace core;

IConn::IConn():eHandler(NULL), dHandler(NULL), bEnable(true){
  proxyId = 0;
}
void IConn::setHandler(ILinkHandler* hdl) {
	dHandler = hdl;
}

void IConn::setLinkEvent(ILinkEvent *ehdl) {
	eHandler = ehdl;
}

ILinkEvent * IConn::getLinkEvent(){
	return eHandler;
}
ILinkHandler *IConn::getHandler() {
	return dHandler;
}
/*
void IConn::bindUid(const std::string &id) {
	uid = id;
	log(Info, "uid bind:", uid, "ip:", sox::addr_ntoa(peerIp), "port:", peerPort);
}

std::string IConn::getUid() const {
	return uid;
}*/

uint32_t IConn::getPeerIp() const {
	return peerIp;
}
int IConn::getPeerPort() const {
	return peerPort;
}
uint32_t IConn::getConnId() const {
	return id;
}

void IConn::setConnId(uint32_t i) {
	id = i;
}

void IConn::setSerialId(uint32_t s){
	serialId = s;
}

uint32_t IConn::getSerialId() const{
	return serialId;
}


bool IConn::isEncrypto() const{
	return false;
}

int IConn::getLocalIp(){
	return 0;
}
