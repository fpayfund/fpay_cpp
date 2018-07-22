#include "RedisAsynWriteClient.h"
#include "RedisAsynResponseLinkHandler.h"
#include "RedisProtocol.h"
#include <string>
#include <stdio.h>
#include "AutoDone.h"

namespace core{
using namespace std;

static uint32_t CHECK_REDIS_CONNECT_TIME_OUT = 0.5*1000;
static uint32_t SEND_TIME_INTERVAL           = 0.1*1000;
RedisAsynWriteClient* RedisAsynWriteClient::client = NULL;

static inline string format_int_to_string(uint32_t i)
{
  char buf[20];
  sprintf(buf,"%u",i);
  return string(buf);
}

RedisAsynWriteClient::RedisAsynWriteClient()
{
    log( Info, "RedisAsynWriteClient::RedisAsynWriteClient()" );
    conn_creater = new InnerConnCreator();
    setClientConnCreator(conn_creater);
    rHandler = new RedisAsynResponseLinkHandler();
      
    connect_check_timer.init(this);
    connect_check_timer.start(CHECK_REDIS_CONNECT_TIME_OUT);
    send_timer.init(this);
    send_timer.start(SEND_TIME_INTERVAL);
}

RedisAsynWriteClient::~RedisAsynWriteClient()
{
    delete rHandler;
    delete conn_creater;
}


void RedisAsynWriteClient::Connect(const std::vector< std::pair<std::string,/*ctl ip*/std::string/*cnc ip*/> >& ips, uint32_t port )
{
    static uint32_t redis_machine_id = 0;
    std::vector< std::pair<std::string,/*ctl ip*/std::string/*cnc ip*/> >::const_iterator it;
    for( it = ips.begin(); it != ips.end(); ++it )
    {
        redis_machine_id++;
        log( Info, "init,Connect to host(%s,%s,%u)",it->first.c_str(), it->second.c_str(),port);

        string ctlhost = it->first + "-" + format_int_to_string(port);
        RedisConnectInfo rconn; 
        rconn.redis_ip = it->first;
        rconn.redis_port = port;
        rconn.connected = true;       
        rconn.conn = createClientConn(it->first,port,rHandler,this);
        host2redisconn_map[ctlhost] = rconn;   
        connid2host_map[rconn.conn->getConnId()] = ctlhost;

        string cnchost = it->second + "-" + format_int_to_string(port);
        rconn.redis_ip = it->second;      
        rconn.conn = createClientConn(it->second,port,rHandler,this);
        host2redisconn_map[cnchost] = rconn;
        connid2host_map[rconn.conn->getConnId()] = cnchost;
   
        machineid2hosts_map[redis_machine_id] = make_pair(ctlhost,cnchost);
    }  
}


void RedisAsynWriteClient::Hset(const std::string& key, const std::string& field, const std::string& value)
{
    RedisDataIf* data = new HsetData(key,field,value); 
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);   
}

void RedisAsynWriteClient::Hmset(const std::string& key,const std::vector<std::pair<std::string,std::string> >& f2vs)
{
    RedisDataIf* data = new HmsetData(key,f2vs);
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);
}

void RedisAsynWriteClient::Hdel(const std::string& key, const std::string& field)
{
    RedisDataIf* data = new HdelData(key,field);
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);
}

void RedisAsynWriteClient::Hincrby(const std::string& key, const std::string& field, const std::string& value)
{
    RedisDataIf* data = new HincrbyData(key, field, value);
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);
}

void RedisAsynWriteClient::Set(const std::string& key, const std::string& value)
{
    RedisDataIf* data = new SetData(key,value);
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);
}

void RedisAsynWriteClient::Sadd(const std::string& key, const std::string& value)
{
    RedisDataIf* data = new SaddData(key,value);
    AutoDone< TrySimpleLock > guard( lock, &TrySimpleLock::Lock, &TrySimpleLock::Unlock );
    sender_queue.push_back(data);
}

bool RedisAsynWriteClient::TimerSend()
{
    if( lock.TryLock() )
    {
        AutoDone< TrySimpleLock > guard( lock, NULL, &TrySimpleLock::Unlock );        
        while( !sender_queue.empty() )
        {
            RedisDataIf* data = sender_queue.front();
            sender_queue.pop_front(); 
            string packet = data->Construct();
            delete data;
     
            std::map<uint32_t/*redis_machine_id*/,std::pair<std::string,std::string> >::iterator it;
            for( it = machineid2hosts_map.begin(); it != machineid2hosts_map.end(); ++it )
            {
                IConn* conn = host2redisconn_map[it->second.first].conn == NULL ? 
                        host2redisconn_map[it->second.second].conn : host2redisconn_map[it->second.first].conn;      
                if( conn )
                {
                    conn->sendBin(packet.c_str(), packet.size(), 0);
                }
            }
        }       
    }
    return true;
}


void RedisAsynWriteClient::eraseConnect(IConn *conn)
{
    string host = connid2host_map[conn->getConnId()];
    connid2host_map.erase(conn->getConnId());
    host2redisconn_map[host].connected = false;
    host2redisconn_map[host].conn = NULL;
    log( Error, "conn(%s) disconnected",host.c_str()); 
    MultiConnManagerImp::eraseConnect(conn);  
}

bool RedisAsynWriteClient::TimerCheckConnect()
{
    //log( Info, "checking Connect,host2redisconn_map size:%Zu",host2redisconn_map.size());
    std::map<std::string,RedisConnectInfo>::iterator it;
    for( it = host2redisconn_map.begin(); it != host2redisconn_map.end(); ++it )
    {    
        if( !it->second.connected )
        {

            IConn* conn = createClientConn(it->second.redis_ip,it->second.redis_port,rHandler,this);   
            if( !conn )
            {
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

