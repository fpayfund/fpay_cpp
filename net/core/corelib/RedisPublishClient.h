#ifndef REDISPUBLISHCLIENT_H
#define REDISPUBLISHCLIENT_H
#include "MultiConnManagerImp.h"
#include "RedisCallbackIf.h"
#include "InnerConn.h"
#include <core/sox/snox.h>
#include "RedisTimer.h"
#include "TrySimpleLock.h"
#include <map>
#include <string>
#include <deque>
namespace core{

class RedisPublishClient : protected MultiConnManagerImp                           
{
    struct RedisConnectInfo
    {
        std::string redis_ip;
        uint32_t    redis_port;    
        bool connected;
        IConn* conn;
        RedisConnectInfo():redis_port(0),connected(false),conn(NULL)
        {}
    };

    struct PublishInfo
    {
        std::string key;
        std::string value;
    };
 
public:
    static RedisPublishClient* Singleton() 
    {
        if( client == NULL ) client = new RedisPublishClient();
        return client; 
    }
    ~RedisPublishClient();
    void Connect(const std::vector<std::string>& ips, uint32_t port );
    size_t Publish(const std::string& key, const std::string& value);

private:
    RedisPublishClient();
    RedisPublishClient(const RedisPublishClient&);
    RedisPublishClient& operator=(const RedisPublishClient&);

    void eraseConnect(IConn *conn);
    virtual bool TimerCheckConnect();
    virtual bool TimerSend();

private:
    static RedisPublishClient* client;

    InnerConnCreator* conn_creater;
    std::map<std::string,RedisConnectInfo> host2redisconn_map;
    std::map<uint32_t/*connid*/,std::string/*hostinfo*/> connid2host_map;
    RedisTimer<RedisPublishClient,&RedisPublishClient::TimerCheckConnect> connect_check_timer;
    RedisTimer<RedisPublishClient,&RedisPublishClient::TimerSend> send_timer; 
    std::deque<PublishInfo> sender_queue;
    TrySimpleLock lock;      
   
};

}

#endif

