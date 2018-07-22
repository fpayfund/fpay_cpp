#ifndef ISERVER_H_
#define ISERVER_H_
#include "common/socketinc.h"
//#include "base_svid.h"
#include "common/int_types.h"
#include "common/core/ilink.h"
#include <string>
#include <vector>
#include "core/sox/logger.h"
namespace server{
	namespace config{
		struct IServerConfig;
	}
}
namespace core {
struct IServerDispatcher;
struct IServerConnFactory;
struct IAppContextEx;
struct ILinkHandler;
struct IClientDaemon;
struct MUUID;

struct IAppContextAware {
protected:
	IAppContextEx *appContext;
public:
	IAppContextAware(): appContext(NULL){}
	virtual ~IAppContextAware() {}
	virtual void setAppContext(IAppContextEx *c){
		appContext = c;
	}
};

struct IServer:	public ILinkHandlerAware{
	
	virtual ~IServer() {}

	virtual void startSV() = 0;

	virtual void hangUp() = 0;

	virtual void wakeUp() = 0;

	virtual void setName(const std::string &st) = 0;

	virtual std::string getName() = 0;
	//获取全名,
	virtual void getFullName(std::string& get_fullname) = 0;

	virtual uint32_t getServerId() = 0;

	virtual std::string getIp() = 0;

	virtual std::vector<uint16_t> getPorts() = 0;

	virtual int getNetType() = 0;
	virtual void setGroupId(uint32_t pGroupId) = 0; /*use TinyXmlServerConfigImp::getGroupId*/
	virtual uint32_t getGroupId() = 0;	
	
	virtual bool setInitStatus(int ) = 0;
};

struct IServerAware{
	virtual ~IServerAware(){}
	virtual void setServer1(IServer *s){
		ss = s;
	}
protected:
	IServer *ss;
};

struct IServerWatcher{
public:
	virtual ~IServerWatcher(){}
	virtual void onServerRegistered() = 0;
};

struct IDaemonServer: public virtual IServer{
	virtual ~IDaemonServer(){}

	virtual void watch(IServerWatcher *w) = 0;
	virtual void revoke(IServerWatcher *w) = 0;

	virtual MUUID getUuid() = 0;
};

struct IDaemonServerAware{
protected:
	IDaemonServer *server;
public:
	IDaemonServerAware():server(NULL){}
	virtual ~IDaemonServerAware(){}
	virtual void setServer(IDaemonServer *s){
		server = s;
		//test, add watcher here
//		server->watch(this);
	}

	virtual IDaemonServer *getDaemonServer(){
		return server;
	}
};

struct IDaemonServerAwareW: public IServerWatcher{
protected:
	IDaemonServer *server;
public:
	virtual ~IDaemonServerAwareW(){}
	virtual void setServer(IDaemonServer *s){
		log(Info, "-------------IDaemonServerAwareW: setServer");
		server = s;
		server->watch(this);
	}
};
}
#endif /*ISERVER_H_*/
