#ifndef REDISCONNMANAGERIMP_H
#define REDISCONNMANAGERIMP_H
#include "MultiConnManagerImp.h"
#include "RedisCallbackIf.h"
#include "InnerConn.h"
#include <core/sox/snox.h>
#include "RedisTimer.h"

namespace core{

class RedisSubscribeClient : protected MultiConnManagerImp                           
{
    struct RedisConnectInfo
    {
        std::string redis_ip;
        uint32_t    redis_port;
        std::string subscribe_key;
        bool connected;
        RedisConnectInfo():connected(false)
        {}
    };

public:
    RedisSubscribeClient(IRedisSubscribeCallback* cb);
    ~RedisSubscribeClient();

    void Subscribe(const std::string& ip, uint32_t port,const std::string& key);
protected:
    void eraseConnect(IConn *conn);
    virtual bool TimerCheckConnect();

private:
    InnerConnCreator* conn_creater;
    std::map<std::string,RedisConnectInfo> host2redisconn_map;
    std::map<uint32_t/*connid*/,std::string/*hostinfo*/> connid2host_map;

    IRedisSubscribeCallback* subscribe_cb;
    RedisTimer<RedisSubscribeClient,&RedisSubscribeClient::TimerCheckConnect> connect_check_timer;
};

}

#endif

