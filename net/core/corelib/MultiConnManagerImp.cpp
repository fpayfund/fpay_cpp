#include "MultiConnManagerImp.h"
#include "common/core/ibase.h"
#include "core/sox/logger.h"
#include <stdio.h>

using namespace core;

MultiConnManagerImp::MultiConnManagerImp():cid(0){

}

MultiConnManagerImp::~MultiConnManagerImp(){
	for (connect_t::iterator it = connects.begin(); it != connects.end(); ++it) {
		delete (*it).second;
	}
}

void MultiConnManagerImp::eraseConnect(IConn *conn) {
	connect_t::size_type sz =connects.erase(conn->getConnId());
	if(sz != 0)
	  delete conn;
}

void MultiConnManagerImp::eraseConnectById(uint32_t id) {
	connect_t::iterator it = connects.find(id);
	if (it != connects.end()) {
		eraseConnect(it->second);
	}
}

IConn *MultiConnManagerImp::getConnectById(uint32_t id) {
	connect_t::iterator it = connects.find(id);
	return it == connects.end() ? NULL : it->second;
}


bool MultiConnManagerImp::dispatchByIds(const std::set<uint32_t> &ids, Sender &resp,  uint32_t exp){

	resp.endPack();
	if(exp != NONEEXP_CID){
		for (std::set<uint32_t>::const_iterator it = ids.begin(); it!= ids.end(); ++it) {
			if(exp != *it){
				IConn *conn = *it < connects.size() ? connects[*it] : NULL;
				if(conn){
					try{
						conn->sendBin(resp.header(), resp.headerSize() + resp.bodySize(), resp.getUri());
					}catch(std::exception &se) {
						conn->setEnable(false);
						toDelete.insert(*it);
						log(Error, "Sox SocketError in multi dispatch ids, err: %s", se.what());
						//            mexp.exps.push_back((*it_c).second);
					}
				}else{
				
					//log(Info, "broadcast connect absence: ");
				}
			}
		}
	}else{
		for (std::set<uint32_t>::const_iterator it = ids.begin(); it!= ids.end(); ++it) {
			IConn *conn = *it < connects.size() ? connects[*it] : NULL;
			if(conn){
				try{
					conn->sendBin(resp.header(), resp.headerSize() + resp.bodySize(), resp.getUri());
				}catch(std::exception &se) {
					conn->setEnable(false);
					toDelete.insert(*it);
					log(Error, "Sox SocketError in multi dispatch ids, err: %s", se.what());
					
				}
			}else{
			
				//log(Info, "broadcast connect absence: ");
			}
		}
	}

	return true;
}


bool MultiConnManagerImp::dispatchById(uint32_t cid, Sender &request) {
	connect_t::iterator it = connects.find(cid);
	if (it != connects.end()) {

		try{
			(*it).second->send(request);
			return true;
		}catch(std::exception &se){
			log(Error, "dispatch by cid:%u err: %s uri:%02x, size:%u", cid, se.what(), request.getUri(), request.bodySize());
			(*it).second->setEnable(false);
			toDelete.insert(cid);

		}
	} else {
		log(Info, "couldnot dispatch response: %u", cid);
	}
	return false;
}

void MultiConnManagerImp::onConnCreate(IConn *conn){
	conn->setConnId(++cid);
	conn->setSerialId(cid);
	connects[conn->getConnId()] = conn;
	conn->setLinkEvent(this);
	fprintf(stderr,"some one connect in...\n");
}
