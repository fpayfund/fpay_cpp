#include "MAbstractServer.h"
#include "core/sox/logger.h"
///#include "common/protocol/const.h"
#include <sstream>

#define NONE_SERVER_ID 0xffffffff
using namespace core;
MAbstractServer::MAbstractServer() {
	_server_id = NONE_SERVER_ID;
}

void MAbstractServer::setName(const std::string &n){
	_name = n;
}

void MAbstractServer::getFullName(std::string& get_fullname)
{
	volatile uint32_t get_server_id=_server_id;
	if(get_server_id == NONE_SERVER_ID)
	{
		return;
	}
	std::stringstream ss;
	ss<<get_server_id;
	get_fullname=_name+"/"+ss.str();
}


