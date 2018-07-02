#ifndef _CLIENT_H_
#define _CLIENT_H_
#include "common/core/iserver.h"
#include "common/core/ibase.h"
#include "server_common/protocol/daemon.h"
#include "common/protocol/prouter.h"
#include "server_common/protocol/prouter.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "server_common/helper/TimerHandler.h"
#include "server_common/protocol/PImRouter.h"
#include "core/sox/mutex.h"
#include "TrySimpleLock.h"
#include <time.h>
#include "doublelinkdefine.h"
#include <deque>
#include "INetSender.h"

namespace doublelink{
using std::map;
using std::vector;
using std::string;
using std::pair;
using std::set;
using std::deque;

class Client: public core::PHClass, 
              public core::IFormTarget, 
              public core::ILinkHandlerAware,
              public core::MultiConnManagerImp,
              public IDispatchResponse
{
protected:
    vector<std::pair<IP_ADDR_T,PORT_T> > unconnect_proxys;
    vector<std::pair<IP_ADDR_T,PORT_T> > connected_proxys;
public:
    Client();
    virtual ~Client();
    
    void startSV(const vector<pair<string,/*ip*/int32_t/*port*/> >& p );
    std::string getServiceFlag();
    void setServiceFlag(const std::string& n); 

    //response or broadcast interface
    virtual void response(const string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj);
    virtual void multicast(uint32_t from_uid, uint32_t channel_id, const set<uint32_t>& uids, uint32_t uri, sox::Marshallable& obj);
    virtual void broadcastByChannel(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj);
    virtual void broadcastBySubChannel( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj);


    void response_nolock(const std::string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj);
    void broadcastByChannel_nolock(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj);
    void broadcastBySubChannel_nolock( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj);
    //extend interface
    void ext_request(const std::string& proxy, uint32_t channel_id,uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t msg_seq, uint8_t need_return, sox::Marshallable& obj,const std::string& dispatch_router_name);

    //query
    void query_by_channel_resp(const std::string& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj);
    void query_by_uid_resp(const std::string& proxy, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj);

    //userinfo
    void report_user_info(uint8_t op ,const user_info_t& user_info);
protected:
    void dispatchByProxy(const PROXY_INFO_T& proxy, core::Sender& send );
    void dispatchByProxy(const PROXY_INFO_T& proxy, uint32_t channel_id, uint32_t uid, uint32_t uri, const std::vector<sox::Marshallable*>& objs);
    //virtual void onConnected(core::IConn* c);//这个事件不一定有，如果connect 立马连接成功，就不会存在这个事件，故不能随便用这个事件
    virtual void eraseConnect(core::IConn *conn);
    virtual void onError(int ev, const char *msg, core::IConn *conn);

    virtual void registerOnConn(const string& proxy,uint32_t connid) {}    
    PROXY_INFO_T getProxyInfoFromConnId( uint32_t cid );

    map<uint32_t,/*connid*/uint32_t> conn_timestamp_map;

    bool init;
private:
    void sendByProxy( const PROXY_INFO_T& proxy, core::Sender& send);
    void sendByRandom( core::Sender& send );
    void sendByChannelRoll(uint32_t channel_id, core::Sender& send);
    core::IConn *connectProxy(const IP_ADDR_T& ip, PORT_T port);
    void connectAllProxy();
    bool dispatch();
    bool linkCheck();
    bool linkStat();
    bool ping();
    //bool checkPingTimeout();

private:
    map<PROXY_INFO_T,uint32_t> proxy2cid;
    map<uint32_t,PROXY_INFO_T> cid2proxy;
    vector<uint32_t/*connid*/> all_cid;
    uint64_t poll;
    LOCAL_INFO_T name;
    
    TimerHandler<Client, &Client::linkCheck> timer_link_check;
    TimerHandler<Client, &Client::dispatch> timer_dispatch;
    TimerHandler<Client, &Client::linkStat> timer_link_stat;
    TimerHandler<Client, &Client::ping> timer_ping;
    //TimerHandler<Client, &Client::checkPingTimeout> timer_check_ping;
    
    deque<pair<PROXY_INFO_T,core::Sender> > sender_queue;
    TrySimpleLock lock;

};

}
#endif

