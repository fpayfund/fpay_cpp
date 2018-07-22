#ifndef ICONFIG_H_
#define ICONFIG_H_
#include <string>
#include <vector>
#include "common/socketinc.h"
#include "common/int_types.h"


namespace core{
struct IConfig{
	virtual void load() = 0;
	
	virtual ~IConfig() { }
	
	void setMyIps(const std::vector<uint32_t> &ips){
		myIp.assign(ips.begin(), ips.end());
	}

	void setMyIp(u_long ip){
		myIp.push_back(ip);
	}

	virtual void setMyPort(int p){
		port = p;
	}
protected:
	std::vector<uint32_t> myIp;
	int port;
};
}
#endif /*ICONFIG_H_*/
