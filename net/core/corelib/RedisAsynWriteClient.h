#ifndef REDISASYNWRITECLIENT_H
#define REDISASYNWRITECLIENT_H
#include "MultiConnManagerImp.h"
#include "RedisCallbackIf.h"
#include "InnerConn.h"
#include <core/sox/snox.h>
#include "RedisTimer.h"
#include "TrySimpleLock.h"
#include "RedisProtocol.h"
#include "RedisAsynResponseLinkHandler.h"

#include <map>
#include <string>
#include <deque>

namespace core{


class RedisDataIf
{
public:
  virtual std::string Construct() = 0;
  virtual ~RedisDataIf() {} //debug 2014-8-11,此处没有，导致通过基类删除对象会导致内存泄露
};

class HsetData : public RedisDataIf
{
public:
  HsetData(const std::string& k, const std::string& f, const std::string& v):    
    key(k),field(f),value(v)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructHset(key,field,value,packet);
    return packet;
  }
private:
  std::string key;
  std::string field;
  std::string value;
};

class SetData : public RedisDataIf
{
public:
  SetData(const std::string& k, const std::string& v):
    key(k),value(v)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructSet(key,value,packet);
    return packet;
  }
private:
  std::string key;
  std::string value;
};


class SaddData : public RedisDataIf
{
public:
  SaddData(const std::string& k, const std::string& v):
    key(k),value(v)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructSadd(key,value,packet);
    return packet;
  }
private:
  std::string key;
  std::string value;
};

class HmsetData : public RedisDataIf
{
public:
  HmsetData(const std::string& k, const std::vector<std::pair<std::string,std::string> >& in):
    key(k),f2vs(in)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructHmset(key,f2vs,packet);
    return packet;
  }
private:
  std::string key;
  std::vector<std::pair<std::string,std::string> > f2vs;
};

class HdelData : public RedisDataIf
{
public:
  HdelData(const std::string& k, const std::string& f):
    key(k),field(f)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructHdel(key,field,packet);
    return packet;
  }
private:
  std::string key;
  std::string field;
};

class HincrbyData : public RedisDataIf
{
public:
  HincrbyData(const std::string& k, const std::string& f, const std::string& v):    
    key(k),field(f),value(v)
  {
  }
  std::string Construct()
  {
    std::string packet;
    RedisProtocol::ConstructHincrby(key,field,value,packet);
    return packet;
  }
private:
  std::string key;
  std::string field;
  std::string value;
};

class RedisAsynWriteClient : protected MultiConnManagerImp                           
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
public:
    static RedisAsynWriteClient* Singleton() 
    {
        if( client == NULL ) client = new RedisAsynWriteClient();
        return client; 
    }
    ~RedisAsynWriteClient();
    void Connect(const std::vector< std::pair<std::string,/*ctl ip*/std::string/*cnc ip*/> >& ips, uint32_t port );
    void Hset(const std::string& key, const std::string& field, const std::string& value);
    void Hmset(const std::string& key,const std::vector<std::pair<std::string,std::string> >& f2vs);
    void Hdel(const std::string& key, const std::string& field);
    void Hincrby(const std::string& key, const std::string& field, const std::string& value);
    void Set( const std::string& key, const std::string& value );
    void Sadd( const std::string& key, const std::string& value );
private:
    RedisAsynWriteClient();
    RedisAsynWriteClient(const RedisAsynWriteClient&);
    RedisAsynWriteClient& operator=(const RedisAsynWriteClient&);

    void eraseConnect(IConn *conn);
    virtual bool TimerCheckConnect();
    virtual bool TimerSend();

private:
    static RedisAsynWriteClient* client;

    InnerConnCreator* conn_creater;
    RedisAsynResponseLinkHandler* rHandler;
    std::map<std::string,RedisConnectInfo> host2redisconn_map;
    std::map<uint32_t/*connid*/,std::string/*hostinfo*/> connid2host_map;
    std::map<uint32_t/*redis_machine_id*/,std::pair<std::string,std::string> > machineid2hosts_map;

    RedisTimer<RedisAsynWriteClient,&RedisAsynWriteClient::TimerCheckConnect> connect_check_timer;
    RedisTimer<RedisAsynWriteClient,&RedisAsynWriteClient::TimerSend> send_timer; 
    std::deque<RedisDataIf*> sender_queue;
    TrySimpleLock lock;      
   
};

}

#endif

