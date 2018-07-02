#ifndef UDPCONNIMP_H_
#define UDPCONNIMP_H_

#include "core/sox/udpsock.h"
#include "common/core/iserver.h"
#include "AbstractConn.h"
#include <stdio.h>
namespace core
{

struct UdpConnIndex
{
  uint32_t ip;
  uint32_t port; 
  UdpConnIndex():port(0)
  {}
  UdpConnIndex(uint32_t Ip, uint32_t Port):ip(Ip),port(Port)
  {}
  UdpConnIndex(const std::string& s_ip, uint32_t Port):ip(inet_addr(s_ip.c_str())),port(Port)
  {}
  uint64_t toindex()
  {
    uint64_t idx = 0;
    idx = ip*(2^32);
    idx += (uint64_t)port;
    fprintf(stderr,"toindex,ip:%u,port:%u,idx:%lu\n",ip,port,idx);
    return idx;
  }
};

struct IUdpClient
{
    virtual uint32_t/*cid*/ connectUpdServer(const std::string& ip, uint32_t port) = 0;
    virtual bool dispatchById(uint32_t cid,uint32_t uri,const sox::Marshallable& obj) = 0;
    virtual bool dispatchById(uint32_t cid,const char* data,size_t sz) = 0;
};

struct IUdpClientAware
{
    IUdpClient* udpClient;
    void setUdpClient(IUdpClient* client)
    {
        udpClient = client;
    }
    IUdpClient* getUdpClient()
    {
        return udpClient;
    }
};

class UdpClientImp: public sox::Udpsock, public AbstractConn, public IUdpClient{
	//xxx TODO size
	char m_buffer[1024];
public:	
	UdpClientImp(int localPort,ILinkHandler* handler);
        
        virtual uint32_t connectUpdServer(const std::string& ip, uint32_t port);
        virtual bool dispatchById(uint32_t cid,uint32_t uri,const sox::Marshallable& obj);
        virtual bool dispatchById(uint32_t cid,const char* data,size_t sz);
protected:
	virtual void sendBin(const char *data, size_t sz, uint32_t uri){
		assert(false);
	}
	virtual void setEncKey(const unsigned char*, size_t){}

	virtual void onRead();
         
	virtual void setTimeout(int){}
private:
       std::map<uint32_t,UdpConnIndex> cid_2_connidx;
       std::map<uint64_t,uint32_t> connidx_2_cid;
       ILinkHandler* dHandler;

};
}

#endif /*UDPCONNIMP_H_*/
