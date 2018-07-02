#include "MultiConnManagerVecImp.h"
#include "common/core/ibase.h"
#include "core/sox/logger.h"

using namespace core;

MultiConnManagerVecImp::MultiConnManagerVecImp():serialId(0){

}

MultiConnManagerVecImp::~MultiConnManagerVecImp(){
	for (connect_t::iterator it = connects.begin(); it != connects.end(); ++it) {
		delete (*it);
		*it = NULL;
	}
}

void MultiConnManagerVecImp::eraseConnect(IConn *conn) {
	if(conn){
		uint32_t id = conn->getConnId();
		if(id < connects.size() && conn == connects[id]){
			freeList.push_back(id);
			connects[id] = NULL;
			delete conn;
		}else{
			log(Error, "eraseConnect: conn not in my connects list, id: %u", id);
		}
	}
}

void MultiConnManagerVecImp::eraseConnectById(uint32_t id) {
	if(id < connects.size() && connects[id]){
		freeList.push_back(id);
		IConn *conn = connects[id];
		connects[id] = NULL;
		delete conn;
	}else{
		log(Error, "eraseConnectById: conn not in my connects list, id: %u", id);
	}
}

IConn *MultiConnManagerVecImp::getConnectById(uint32_t id) {
	return id < connects.size() ? connects[id] : NULL;
}

bool MultiConnManagerVecImp::dispatchByIds(const std::set<uint32_t> &ids, Sender &resp,  uint32_t exp){
//#ifndef WIN32
//  struct timeval enter_time;  
//  struct timeval middle_time;
//  struct timeval leave_time;
//  gettimeofday(&enter_time, NULL); 
//#endif

	resp.endPack();
//#ifndef WIN32
//  gettimeofday(&middle_time, NULL);
//  int diff_tick = (middle_time.tv_sec - enter_time.tv_sec)*1000000 + (middle_time.tv_usec - enter_time.tv_usec);
//  m_send_tick1 += diff_tick;
//#endif

	if(exp != NONEEXP_CID){
		for (std::set<uint32_t>::const_iterator it = ids.begin(); it!= ids.end(); ++it) {
			if(exp != *it){
				IConn *conn = *it < connects.size() ? connects[*it] : NULL;
				if(conn){
					try{
						conn->sendBin(resp.header(), resp.headerSize() + resp.bodySize(), resp.getUri());
						//conn->sendBin(resp.data(), resp.size(), resp.getUri());
					}catch(std::exception &se) {
						conn->setEnable(false);
						toDelete.insert(*it);
						log(Error, "Sox SocketError in multi dispatch ids, err:", se.what());
					//			mexp.exps.push_back((*it_c).second);
					}
				}else{
					//xxx todo
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
					//			mexp.exps.push_back((*it_c).second);
				}
			}else{
				//xxx todo
				//log(Info, "broadcast connect absence: ");
			}
		}
	}
//#ifndef WIN32
//  gettimeofday(&leave_time, NULL);
//  int diff_tick2 = (leave_time.tv_sec - enter_time.tv_sec)*1000000 + (leave_time.tv_usec - enter_time.tv_usec);
//  m_send_tick2 += diff_tick2;
//#endif
	return true;
}

bool MultiConnManagerVecImp::dispatchById(uint32_t cid, Sender &resp) {
	IConn *conn = getConnectById(cid);
	if(conn){
		try{
			conn->send(resp);
			return true;
		}catch(std::exception &se) {
			log(Error, "dispatchById Sox SocketError in dispatch id, err: %s", se.what());
			conn->setEnable(false);
      toDelete.insert(cid);
			//			mexp.exps.push_back((*it_c).second);
		}
	}else{
		//xxx todo
		toDelete.insert(cid);
		//log(Info, "broadcast connect absence: ");
	}
	return false;
}

void MultiConnManagerVecImp::onConnCreate(IConn *conn){
	uint32_t fid;
	if(freeList.empty()){
		fid = (uint32_t)connects.size();
		connects.push_back(conn);
	}else{
		fid = freeList.front();
		freeList.pop_front();
		connects[fid] = conn;
	}
	conn->setConnId(fid);
	conn->setSerialId(++serialId);
	conn->setLinkEvent(this);
}
