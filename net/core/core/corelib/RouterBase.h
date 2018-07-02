#ifndef ROUTER_BASE_H
#define ROUTER_BASE_H
#include <iostream>
#include <string>
#include "common/packet.h"
#include "common/res_code.h"
namespace core{
struct IPacketGc;
struct IConn;
struct IServerIdDispatcher;
struct IPacketBase{
	virtual ~IPacketBase(){}
	virtual int emit(core::IConn *conn, core::IServerIdDispatcher* ) = 0;
	virtual void gc(IPacketGc *) = 0;
};

struct RouterBase: public sox::Marshallable, public IPacketBase{
	std::string from;
	uint8_t ttl;
	uint32_t ruri;
	uint16_t resCode;
	std::string load;//最原始的消息包
	RouterBase():ttl(2), resCode(RES_SUCCESS){}

	virtual void marshal(sox::Pack &pk) const{
		pk.push_uint8(ttl);
		pk << from << ruri << resCode;
		pk.push_varstr32(load.data(), load.size());

	}

	virtual void unmarshal(const sox::Unpack &up){
		ttl = up.pop_uint8();
		up >> from >> ruri >> resCode;
		load = up.pop_varstr32();
	}

	void packLoad(const sox::Marshallable &obj){
		sox::PackBuffer pb;
		sox::Pack pk(pb);
		pk << obj;
		load.assign(pk.data(), pk.size());
	}

	//add by kdjie, 2009.12.16
	void packLoad(const std::string &str)
	{
		load.assign(str.data(), str.size());
	}
	//add end
	
};
}
#endif

