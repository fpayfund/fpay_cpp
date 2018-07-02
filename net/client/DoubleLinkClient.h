#ifndef DOUBLELINK_CLIENT_H_
#define DOUBLELINK_CLIENT_H_
#include "Client.h"
#include "doublelink_protocol.h"
#include "INetObserver.h"
namespace doublelink{

class DoubleLinkClient: public Client 
{

public:
    DECLARE_FORM_MAP

    DoubleLinkClient();
    ~DoubleLinkClient();
    
    void setNetObserver(INetObserver* observer);
    void subscribe(const set<pair<uint32_t,uint32_t> >& cset, const set<pair<uint32_t,uint32_t> >& backup_cset);
    void unsubscribe(const set<pair<uint32_t,uint32_t> >& cset, const set<pair<uint32_t,uint32_t> >& backup_cset );
    //request
    void onRequestRoute(PRequestRouter* r, core::IConn* c);
    //extend if
    void onExtResponseRoute(PExtResponseRouter* r, core::IConn* c);
    //ping response
    void onPingResponse(PPingResp* p, core::IConn* c);

    void onQueryByChannelReq(PQueryByChannelReq* r, core::IConn* c);    
    void onQueryByUidReq(PQueryByUidReq* r, core::IConn* c);
    void onEndSessionReq(PEndSessionReq* r, core::IConn* c);
    void onThriftNotice(PThriftNoticeRouter* r, core::IConn* c);
    void onEndSessionForce(PEndSessionForceRouter* r, core::IConn* c);

protected:
    virtual void registerOnConn(const string& proxy,uint32_t connid); 
    
private:
    set<pair<uint32_t,uint32_t> > channel_sets;
    set<pair<uint32_t,uint32_t> > backup_channel_sets;
    INetObserver* net_observer;

};
}
#endif

