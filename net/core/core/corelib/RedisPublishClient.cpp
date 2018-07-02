#include "RedisPublishClient.h"
#include "RedisAsynResponseLinkHandler.h"
#include "RedisProtocol.h"
#include <string>
#include <stdio.h>
#include "AutoDone.h"

namespace core{
using namespace std;

static uint32_t CHECK_REDIS_CONNECT_TIME_OUT = 0.5*1000;
static uint32_t SEND_TIME_INTERVAL           = 0.1*1000;
static size_t ALERT_QUEUE_SIZE               = 500;
static size_t MAX_QUEUE_SIZE                 = 5000;
RedisPublishClient* RedisPublishClient::client = NULL;

static inline string format_int_to_string(uint32_t i)
{
  char buf[20];
  sprintf(buf,"%u",i);
  return string(buf);
}

RedisPublishClient::RedisPublishClient()
{
    log( Info, "RedisPublishClient::RedisPublishClient()" );
    conn_creater = new InnerConnCreator();
    setClientConnCreator(conn_creater);
    
    connect_check_timer.init(this);
    connect_check_timer.start(CHECK_REDIS_CONNECT_TIME_OUT);
    send_timer.init(this);
    send_timer.start(SEND_TIME_INTERVAL);
}

RedisPublishClient::~RedisPublishClient()
{
    delete conn_creater;
}


void RedisPublishClient::Connect(const std::vector<std::string>& ips, uint32_t port )
{

    std::vector<std::string>::const_iterator it;
    for( it = ips.begin(); it != ips.end(); ++it )
    {
        log( Info, "init,Connect to host(%s,%u)",(*it).c_str(),port);
        string host = *it + "-" + format_int_to_string(port);
        RedisConnectInfo rconn;
        rconn.redis_ip = *it;
        rconn.redis_port = port;
        rconn.connected = true;

        RedisAsynResponseLinkHandler* rHandler = new RedisAsynResponseLinkHandler();
        IConn* conn = createClientConn(*it,port,rHandler,this);
        rconn.conn = conn; 

        host2redisconn_map[host] = rconn;   
        connid2host_map[conn->getConnId()] = host;   
    }  
}


size_t RedisPublishClient::Publish(const std::string& key, const std::string& value)
{
    size_t ret;
    PublishInfo pubinfo;  
    pubinfo.key = key;
    pubinfo.value = value; 
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    ret = sender_queue.size();
    if( ret > ALERT_QUEUE_SIZE )
    {
        log( Warn, "Publish queue size:%Zu", ret );
    }
    if( ret >= MAX_QUEUE_SIZE )
    {
        log( Warn, "Publish queue size achive max size,drop");
        return ret;
    }    
    sender_queue.push_back(pubinfo);   
    return ret;
}

bool RedisPublishClient::TimerSend()
{
    if( lock.TryLock() )
    {
        //log( Error, " RedisPublishClient::TimerSend size:%Zu",sender_queue.size());
        AutoDone< TrySimpleLock > guard( lock, NULL, &TrySimpleLock::Unlock );        
        while( !sender_queue.empty() )
        {
            PublishInfo pubinfo = sender_queue.front();
            sender_queue.pop_front(); 
            string publish_packet;
            RedisProtocol::ConstructPublish(pubinfo.key,pubinfo.value,publish_packet);

            std::map<std::string,RedisConnectInfo>::iterator it;
            bool send_success = false;
            for( it = host2redisconn_map.begin(); it != host2redisconn_map.end(); ++it )
            {       
                IConn* conn = it->second.conn;
                if( conn )
                {                    
                    conn->sendBin(publish_packet.c_str(), publish_packet.size(), 0);
                    send_success = true;
                    break;
                }
            }
            if( !send_success )
            {
                log( Info, "all conn is NULL, publish key:%s,value:%s failed",pubinfo.key.c_str(),pubinfo.value.c_str());
            }
        }       
    }
    else
    {
      log( Error, " RedisPublishClient::TimerSend trylock failed ");     
    }
    return true;
}


void RedisPublishClient::eraseConnect(IConn *conn)
{
    ILinkHandler* rHandler = conn->getHandler();
    delete rHandler;
    string host = connid2host_map[conn->getConnId()];
    connid2host_map.erase(conn->getConnId());
    host2redisconn_map[host].connected = false;
    host2redisconn_map[host].conn = NULL;
    log( Error, "conn(%s) disconnected",host.c_str()); 
    MultiConnManagerImp::eraseConnect(conn);  
}

bool RedisPublishClient::TimerCheckConnect()
{
    //log( Info, "checking Connect,host2redisconn_map size:%Zu",host2redisconn_map.size());
    std::map<std::string,RedisConnectInfo>::iterator it;
    for( it = host2redisconn_map.begin(); it != host2redisconn_map.end(); ++it )
    {    
        if( !it->second.connected )
        {
            RedisAsynResponseLinkHandler* rHandler = new RedisAsynResponseLinkHandler();
            IConn* conn = createClientConn(it->second.redis_ip,it->second.redis_port,rHandler,this);   
            if( !conn )
            {
                delete rHandler;
                log( Error, "create conn to(%s,%u) failed",it->second.redis_ip.c_str(),it->second.redis_port);
                continue;
            }
            else
            {
                log( Info, "create conn to(%s,%u) success",it->second.redis_ip.c_str(),it->second.redis_port);
            }  
            it->second.connected = true;         
            it->second.conn = conn;
            connid2host_map[conn->getConnId()] = it->second.redis_ip + "-" + format_int_to_string(it->second.redis_port);
        }     
    }
    return true;
} 

}

