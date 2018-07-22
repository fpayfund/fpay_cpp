#include "RedisSubscribeClient.h"
#include "RedisSubscribeLinkHandler.h"
#include "RedisProtocol.h"
#include <string>
#include <stdio.h>

namespace core{
using namespace std;
static uint32_t CHECK_REDIS_CONNECT_TIME_OUT = 1*1000;
static inline string format_int_to_string(uint32_t i)
{
  char buf[20];
  sprintf(buf,"%u",i);
  return string(buf);
}

RedisSubscribeClient::RedisSubscribeClient(IRedisSubscribeCallback* cb):
  subscribe_cb(cb)
{
    conn_creater = new InnerConnCreator();
    setClientConnCreator(conn_creater);
    
    connect_check_timer.init(this);
    connect_check_timer.start(CHECK_REDIS_CONNECT_TIME_OUT);
}

RedisSubscribeClient::~RedisSubscribeClient()
{
}

void RedisSubscribeClient::Subscribe(const std::string& ip, 
                                                uint32_t port,
                                                const std::string& key)
{
    RedisConnectInfo rconn;
    rconn.redis_ip = ip;
    rconn.redis_port = port;
    rconn.subscribe_key = key; 
    rconn.connected = true;
    
    RedisSubscribeLinkHandler* rHandler = new RedisSubscribeLinkHandler(subscribe_cb);
    IConn* conn = createClientConn(ip,port,rHandler,this);    

    string subscribe_packet;
    RedisProtocol::ConstructSubscribe(key, subscribe_packet);
   
    conn->sendBin(subscribe_packet.c_str(), subscribe_packet.size(), 0);

    string host = ip + "-" + format_int_to_string(port);
    host2redisconn_map[host] = rconn;
    connid2host_map[conn->getConnId()] = host;        
}


void RedisSubscribeClient::eraseConnect(IConn *conn)
{
    ILinkHandler* rHandler = conn->getHandler();
    delete rHandler;
    string host = connid2host_map[conn->getConnId()];
    connid2host_map.erase(conn->getConnId());

    host2redisconn_map[host].connected = false;
    MultiConnManagerImp::eraseConnect(conn); 
   
}

bool RedisSubscribeClient::TimerCheckConnect()
{
    std::map<std::string,RedisConnectInfo>::iterator it;
    for( it = host2redisconn_map.begin(); it != host2redisconn_map.end(); ++it )
    {    
        if( !it->second.connected )
        {
            RedisSubscribeLinkHandler* rHandler = new RedisSubscribeLinkHandler(subscribe_cb);
            IConn* conn = createClientConn(it->second.redis_ip,it->second.redis_port,rHandler,this);

            string subscribe_packet;
            RedisProtocol::ConstructSubscribe(it->second.subscribe_key, subscribe_packet);
            if( !conn )
            {
                log( Info, "create conn to(%s,%u) failed",it->second.redis_ip.c_str(),it->second.redis_port);
                delete rHandler;
                continue;
            }
            conn->sendBin(subscribe_packet.c_str(), subscribe_packet.size(), 0);
            it->second.connected = true;
            connid2host_map[conn->getConnId()] = it->second.redis_ip + "-" + format_int_to_string(it->second.redis_port);   
        }     
    }
    return true;
} 

}

