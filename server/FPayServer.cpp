#include "FPayServer.h"
#include "common/core/form.h"
#include "common/core/ilink.h"
#include "core/sox/logger.h"
#include "core/sox/sockethelper.h"

using namespace core;
//using namespace sdaemon;



struct AutoFalse{
	bool &b;
	AutoFalse(bool &x):b(x){}
	~AutoFalse(){
		b = false;
	}
};

ServerSocketHelper::ServerSocketHelper(FPayServer *d, SOCKET so):sox::ServerSocket(so), imp(d)
{    
}

void ServerSocketHelper::onAccept(SOCKET so, u_long ip, int port)
{
	imp->onAccept(so, ip, port);
}

void ServerSocketHelper::start()
{
	select(0, sox::SEL_READ);
}

FPayServer::~FPayServer()
{
	for(std::vector<ServerSocketHelper *>::iterator it = helpers.begin(); it != helpers.end(); ++it)
	{
		delete *it;
	}
}

ServerSocketHelper *FPayServer::create(const char* ip, uint16_t p, uint16_t &cur)
{
	sox::Sockethelper so;
	so.socket();
	so.setreuse();
	so.setblocking(false);
	so.setnodelay();
	so.bind(p, ip);

	return new ServerSocketHelper(this, so.detach());
}


ServerSocketHelper *FPayServer::createHelper(const char *ip)
{
	if(!expectPorts.empty())
	{        
		try
		{
			uint16_t tmp;
			ServerSocketHelper *ret = create(ip, expectPorts.front(), tmp);

			curports.push_back(expectPorts.front());
			log(Info, "create listen port %d success", expectPorts.front());

			expectPorts.erase(expectPorts.begin());
			return ret;
		}
		catch(sox::socket_error se)
		{
			log(Info, "alloc port conflict port: %u", expectPorts.front());
			expectPorts.erase(expectPorts.begin());
		}
	}

	return NULL;
}


std::vector<uint16_t> FPayServer::getPorts()
{
	return curports;
}


std::string FPayServer::getIp()
{
	return ip;
}

void FPayServer::startSV()
{
	for( uint32_t i = 0; i < expectPorts.size(); i++ )
	{
		log( Info, "FPayServer::StartSv, expect port:%u", expectPorts[i]);
	}
	while(!expectPorts.empty())
	  helpers.push_back(createHelper(NULL));
	if( helpers.empty() )
	{
		log(Error,"FPayServer::StartSV,failed" );
	}
	for (std::vector<ServerSocketHelper *>::iterator it = helpers.begin(); it
				!= helpers.end(); ++it) 
	{
		(*it)->start();
	}

}
void FPayServer::onAccept(SOCKET so, u_long ip, int port)
{
	log(Info, "Accept from from :%s : %d", sox::addr_ntoa(ip).data(), port);
	connManager->createServerConn(so, ip, port, handler, getLinkEvent());    
}


FPayServer::FPayServer()
{
	//recoverSeq = 100;	
	setName("fpay_server_node");
}

