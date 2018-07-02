#include "UdpConnImp.h"
#include "core/sox/logger.h"
#include "common/core/sender.h"

using namespace core;
using namespace sox;

UdpClientImp::UdpClientImp(int localPort,ILinkHandler* handler):
Udpsock(localPort) {
        dHandler = handler;
	select(0, SEL_READ);
}

void UdpClientImp::onRead() {
	socklen_t solen_dummy = sizeof(sox::ipaddr_type);

	ipaddr_type addr;

	memset(&addr, 0, sizeof(addr));

	int m_size = recvfrom(m_buffer, sizeof(m_buffer), &addr, &solen_dummy);

	u_long ip = addr.sin_addr.s_addr;
	int port = ntohs(addr.sin_port);
        UdpConnIndex connidx(ip,port);
        std::map<uint64_t,uint32_t>::iterator idx2cid_it;
        //fprintf(stderr,"onread,ip:%u,port:%d,connidx:%lu\n",addr.sin_addr.s_addr,ntohs(addr.sin_port),connidx.toindex());
        idx2cid_it = connidx_2_cid.find(connidx.toindex());
        if( idx2cid_it == connidx_2_cid.end() )
        {
          log( Error,"no conn to server(%s,%d)",inet_ntoa(addr.sin_addr),port);
          return;
        }
            
        try{
                dHandler->onData(m_buffer, m_size, this, UDP_UNCODE);
        }catch(std::exception & ex){
                log(Error, "handle proc error:%s", ex.what());                
        }

}

uint32_t UdpClientImp::connectUpdServer(const std::string& ip, uint32_t port)
{
    uint32_t connid = 0;
    UdpConnIndex connidx(ip,port);
    //fprintf(stderr,"connectUpdServer,ip:%s,port:%d,connidx:%lu\n",ip.c_str(),port,connidx.toindex());

    std::map<uint64_t,uint32_t>::iterator idx2cid_it;    
    idx2cid_it = connidx_2_cid.find(connidx.toindex());
    if( idx2cid_it == connidx_2_cid.end() )
    {
        try
        {
            udp_connect(ip,port);
            static uint32_t sConnId = 0;
            sConnId++;
            connid = sConnId;
            cid_2_connidx[connid] = connidx;
            connidx_2_cid[connidx.toindex()] = connid; 
        }
        catch(...)
        {
            log( Info," connect to udp server(%s,%u) failed", ip.c_str(), port );
        }
    }
    else
    {
        connid = idx2cid_it->second;
    }

    return connid;
}


bool UdpClientImp::dispatchById(uint32_t cid,uint32_t uri,const sox::Marshallable& obj)
{
    Sender sender(uri,obj);
    sender.endPack();
    return dispatchById(cid,sender.header(),sender.headerSize()+sender.bodySize());
}

bool UdpClientImp::dispatchById(uint32_t cid,const char* data,size_t sz)
{
    std::map<uint32_t,UdpConnIndex>::iterator cid2connidx_it;
    cid2connidx_it = cid_2_connidx.find(cid);
    if( cid2connidx_it == cid_2_connidx.end() )
    {
        log( Error,"not found conn:%u",cid);
        return false;        
    }    
    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    to_addr.sin_addr.s_addr = cid2connidx_it->second.ip;
    to_addr.sin_port =  htons(cid2connidx_it->second.port);    

    //fprintf(stderr,"dispatchById,ip:%u,port:%d,connidx:%lu\n",to_addr.sin_addr.s_addr,cid2connidx_it->second.port,cid2connidx_it->second.toindex());    
    sendto(data,sz,&to_addr,sizeof(to_addr));    
    return true;
}


