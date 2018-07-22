#ifndef ILINK_PROTOCOL_H
#define ILINK_PROTOCOL_H

#include "common/socketinc.h"
#include "common/blockbuffer.h"
#include "common/int_types.h"
//#include "base_svid.h"
#include <string>
#include <set>
#include <map>
#include <stdexcept>
#include <vector>
#include <utility>
#define TCP_ENCODE 0
#define TCP_UNCODE 1
#define UDP_ENCODE 2
#define UDP_UNCODE 3
#define NONEEXP_CID 0xffffffff

namespace sox{
	struct Marshallable;
}

namespace protocol
{
	struct NetworkProxyInfo;
	struct IProxyTestResultCallBack;
}

namespace core {
class Request;
struct IContext;
struct IConn;
struct Sender;

struct ILinkHandler {
	//callback funcs
	//virtual void	onEvent(int ev, const char* msg, class IConn* from) = 0;
	virtual ~ILinkHandler() {
	}
	virtual int onData(const char*, size_t, IConn *conn, int type = 0) = 0;

	virtual void setPackLimit(uint32_t sz) = 0;
};

struct ILinkHandlerAware{
	ILinkHandler *handler;
	ILinkHandlerAware():handler(NULL){}
	virtual ~ILinkHandlerAware(){}
	virtual void setLinkHandler(ILinkHandler *h){
		handler = h;
	}
	virtual ILinkHandler *getLinkHandler(){
		return handler;
	}
};

struct IProcStat{
	virtual ~IProcStat(){}

	virtual uint32_t getAndRstIntevel() = 0;

	virtual uint64_t getTotal() = 0;
};

struct IProcStatAware{
	IProcStat *stat;
	virtual ~IProcStatAware(){}
	virtual void setProcStat(IProcStat *p){
		stat = p;
	}
};

struct ILinkEvent {
	virtual ~ILinkEvent() {
	}
	virtual void onConnected(IConn *conn) = 0;
	virtual void onClose(IConn *conn) = 0;
	//主动关闭
	virtual void onInitiativeClose(IConn *conn) = 0;
	virtual void onError(int ev, const char *msg, IConn *conn) = 0;
	virtual void onTimeout(IConn *conn) = 0;
	virtual void updateRecvTime(){}
};

struct IConn {
	// flush : 关闭之前, 刷空缓存
	//virtual void release(bool flush) throw () = 0;
	//virtual std::string getuid() const = 0;
	IConn();
	
	virtual ~IConn() {
	}

	virtual void send(Sender &resp) = 0;

	virtual void sendBin(const char *data, size_t sz, uint32_t uri) = 0;

	virtual void setTimeout(int tm) = 0;

	virtual void *getData() = 0;
	//virtual void init_() = 0;
	//virtual void connect_(const std::string& ip, unsigned port) = 0;
	//virtual void close_() = 0;
	virtual void setEncKey(const unsigned char *key, size_t len) = 0;

	void setHandler(ILinkHandler* hdl) ;
	void setLinkEvent(ILinkEvent *ehdl);

	ILinkEvent *getLinkEvent();
	ILinkHandler *getHandler() ;
	/*void bindUid(const std::string &id);
	std::string getUid() const ;*/
	uint32_t getPeerIp() const ;
	int getPeerPort() const ;
	uint32_t getConnId() const ;
	void setConnId(uint32_t i);

  uint32_t getProxyId() const { return proxyId; }
  void setProxyId(uint32_t i) { proxyId = i; }

	void setSerialId(uint32_t s);
	uint32_t getSerialId() const;

	void setEnable(bool be = false){
		bEnable = be;
	}
	
	bool isEnable() const{
		return bEnable;
	}

	virtual bool isEncrypto() const;

	virtual int getLocalIp();

	virtual int getConnType()
	{
		//default type tcp
		return 1;
	}

	virtual uint32_t getConnId(void)
	{
		return id;
	}

protected:
	ILinkEvent *eHandler;
	ILinkHandler *dHandler;
	//std::string uid;
	uint32_t id;
	uint32_t peerIp;
	uint32_t serialId;
  uint32_t proxyId;
	int peerPort;
	bool bEnable;
};



struct IConnDispatcher {
	virtual ~IConnDispatcher() {
	}
	virtual bool dispatchById(uint32_t cid, Sender &) = 0;
	virtual bool dispatchByIds(const std::set<uint32_t> &ids, Sender &, uint32_t exp = NONEEXP_CID) = 0;
};

struct CreateCallback{
	virtual ~CreateCallback(){}
	virtual void onConnCreate(IConn *conn) = 0;
};

struct IClientConnCreator{
public:
	virtual ~IClientConnCreator() {}
	virtual IConn *createConnect(const std::string& ip, uint32_t port, ILinkHandler *ih, ILinkEvent *ie, CreateCallback *) = 0;
};

struct IProxyTestConnCreator{
public:
	virtual ~IProxyTestConnCreator() {}
	virtual IConn *createProxyTestConnect(const protocol::NetworkProxyInfo& proxyInfo, protocol::IProxyTestResultCallBack* pCallback) = 0;
};

struct IServerConnCreator{
public:
	virtual ~IServerConnCreator(){}
	virtual IConn *creatConnect(SOCKET, uint32_t ip, int port, ILinkHandler *ih, ILinkEvent *ie, CreateCallback *) = 0;
};

struct IConnManager: public IConnDispatcher, public CreateCallback{
protected:
	IClientConnCreator *clientCreator;
	IServerConnCreator *serverCreator;
public:
	IConnManager():clientCreator(NULL), serverCreator(NULL){}
	~IConnManager();

	virtual void eraseConnect(IConn *conn) = 0;
	virtual void eraseConnectById(uint32_t id) = 0;

	virtual IConn *getConnectById(uint32_t id) = 0;

	virtual int getLocalIp() const {return 0;}

	virtual void setClientConnCreator(IClientConnCreator *cc){
		clientCreator = cc;
	};

	virtual void setServerConnCreator(IServerConnCreator *sc){
		serverCreator = sc;
	}

	virtual IConn *createServerConn(SOCKET, uint32_t ip, int port, ILinkHandler *iH, ILinkEvent *iE) = 0;

  virtual IConn *createClientConn(const std::string& ip, uint32_t port, ILinkHandler *iH, ILinkEvent *iE) = 0;

public:
  uint32_t m_send_tick1;
  uint32_t m_send_tick2;
};


struct IDelayDelConn{
	virtual ~IDelayDelConn(){}
	virtual void DelayDelConn(IConn *conn) = 0;
	//virtual void DelayDelConnByID(uint32_t cid) = 0;
};


struct IDelayDelConnAware{
protected:
	IDelayDelConn *delayDelConn;

public:
	virtual void setDelayDelConn(IDelayDelConn *d){
		delayDelConn = d;
	}
};

struct IServerIdDispatcher{
	virtual ~IServerIdDispatcher(){}
//	virtual bool dispatchByServerId(uint32_t serverId, uint32_t uri, sox::Marshallable &) = 0;
	virtual bool dispatchByServerIdSV(uint32_t serverId, uint32_t uri, sox::Marshallable &) = 0;
	virtual uint32_t dispatchToServerRandom(const std::string &name, uint32_t uri, sox::Marshallable &) = 0;
	virtual void dispatchToServers(const std::string &name, uint32_t uri, sox::Marshallable &) = 0;

	// 轮询的方式平均发送
	virtual void DispatchToServerRoundRobin(const std::string& strSuffix, uint32_t uUri, sox::Marshallable& obj){};

};

struct IServerIdDispatcherAware{
protected:
	IServerIdDispatcher *sDispatcher;
public:
	virtual ~IServerIdDispatcherAware(){}
	virtual void setServerIdDispatcher(IServerIdDispatcher *s){
		sDispatcher = s;
	}
};

struct IConnManagerAware {
protected:
	IConnManager *connManager;
public:
	IConnManagerAware() :
		connManager(NULL) {
	}
	virtual ~IConnManagerAware() {
	}
	virtual void setConnManager(IConnManager *c) {
		connManager = c;
	}

	IConnManager *getConnManager() const{
		return connManager;
	}
};

struct IConnDispatcherAware{
protected:
	IConnDispatcher *dispatcher;
public:
	IConnDispatcherAware() : dispatcher(NULL) {
	  }
	  virtual ~IConnDispatcherAware() {
	  }
	  virtual void setConnDispatcher(IConnDispatcher *c) {
		  dispatcher = c;
	  }
};

}
#endif

