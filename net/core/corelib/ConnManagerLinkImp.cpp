#include "ConnManagerLinkImp.h"
#include "core/sox/logger.h"
using namespace core;

#define DELAY_DEL_CONN_TIME 10

void ConnManagerLinkImp::onConnected(IConn *conn){

}
void ConnManagerLinkImp::onClose(IConn *conn){
	log(Debug, "[ConnManagerLinkImp::onClose] conn id:%u",conn->getConnId());
	eraseConnect(conn);
}
//Ö÷¶¯¹Ø±Õ
void ConnManagerLinkImp::onInitiativeClose(IConn *conn){
	if(conn)
		log(Debug, "[ConnManagerLinkImp::onInitiativeClose] conn id:%u",conn->getConnId());
	eraseConnect(conn);
}
void ConnManagerLinkImp::onError(int ev, const char *msg, IConn *conn){
	log(Debug, "[ConnManagerLinkImp::onError]: Event = %d, msg = %s, cid = %d", ev, msg, conn ? conn->getConnId() : - 1);
	eraseConnect(conn);
}

void ConnManagerLinkImp::onTimeout(IConn *conn){
	log(Debug, "[ConnManagerLinkImp::onTimeout]: connid = %d", conn ? conn->getConnId() : -1);
	eraseConnect(conn);
}

LazyDelConnManager::LazyDelConnManager(){
	select_timeout(DELAY_DEL_CONN_TIME);
}

void LazyDelConnManager::handle(int){
	if(!toDelete.empty()){
		for(std::set<uint32_t>::iterator it = toDelete.begin(); it != toDelete.end(); ++it){
			log(Notice, "[LazyDelConnManager::handle]: delete conn = %d", *it);
			IConn *conn = getConnectById(*it);
			if(conn && !conn->isEnable()){
				eraseConnect(conn);
			}
		}
		log(Notice, "[LazyDelConnManager::handle]: timely clear %d conn", toDelete.size());
		toDelete.clear();
	}
	select_timeout(DELAY_DEL_CONN_TIME);
}

void LazyDelConnManager::DelayDelConn(IConn *conn){
	if (conn){
		conn->setEnable(false);
		toDelete.insert(conn->getConnId());
	}
}

/*
void LazyDelConnManager::DelayDelConnByID(uint32_t cid){
	toDelete.insert(cid);
}*/


IConn *LazyDelConnManager::createServerConn(SOCKET so, uint32_t ip, int port, ILinkHandler *h, ILinkEvent *eH){
	IConn *conn = serverCreator->creatConnect(so, ip, port, h, eH, this);
	return conn;
}

IConn *LazyDelConnManager::createClientConn(const std::string& ip, uint32_t port, ILinkHandler *iH, ILinkEvent *eH){
	IConn *conn = clientCreator->createConnect(ip, port, iH, eH, this);
	return conn;
}
