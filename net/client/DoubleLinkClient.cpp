#include "DoubleLinkClient.h"
#include "common/core/form.h"
#include "core/sox/sockethelper.h"
#include "core/sox/logger.h"
#include "core/sox/udpsock.h"
#include "core/corelib/AbstractConn.h"
#include "core/corelib/InnerConn.h"

#include <signal.h>
#include <unistd.h>

using namespace core;
using namespace sox;
using namespace sdaemon;
using namespace protocol;

namespace doublelink {
	using std::set;
	using std::vector;
	using std::pair;

	BEGIN_FORM_MAP(DoubleLinkClient)
		ON_LINK(PRequestRouter, &DoubleLinkClient::onRequestRoute)
		ON_LINK(PPingResp, &DoubleLinkClient::onPingResponse)
		ON_LINK(PExtResponseRouter, &DoubleLinkClient::onExtResponseRoute)
		ON_LINK(PQueryByChannelReq, &DoubleLinkClient::onQueryByChannelReq)
		ON_LINK(PQueryByUidReq, &DoubleLinkClient::onQueryByUidReq)
		ON_LINK(PEndSessionReq, &DoubleLinkClient::onEndSessionReq)
		ON_LINK(PThriftNoticeRouter, &DoubleLinkClient::onThriftNotice)
		ON_LINK(PEndSessionForceRouter, &DoubleLinkClient::onEndSessionForce)

		END_FORM_MAP()

		const uint32_t TIMER_SUBSCRIBE_CHECK  = 1000 * 1;

	DoubleLinkClient::DoubleLinkClient():
		net_observer(NULL)
	{
	}

	DoubleLinkClient::~DoubleLinkClient()
	{
	}

	void DoubleLinkClient::setNetObserver(INetObserver* observer)
	{
		net_observer = observer;
	}

	void DoubleLinkClient::subscribe(const set<pair<uint32_t,uint32_t> >& cset, const set<pair<uint32_t,uint32_t> >& backup_cset )
	{
		channel_sets = cset;
		backup_channel_sets = backup_cset;
		vector<std::pair<IP_ADDR_T,PORT_T> >::iterator it;
		for( it = connected_proxys.begin(); it != connected_proxys.end(); ++it )
		{
			ChannelsSubscribeRegister reg;
			reg.service_flag = getServiceFlag();
			reg.to = format_proxy_info( it->first, it->second );

			set<pair<uint32_t,uint32_t> >::iterator cit;
			for(cit = channel_sets.begin(); cit != channel_sets.end(); ++cit )
			{
				log( Info, "DoubleLinkClient::subscribe,service:%s,channel_set(%d,%d) register to proxy(%s,%d)",
						getServiceFlag().c_str(),cit->first,cit->second,it->first.c_str(),it->second);
				ChannelInterval interval;
				interval.begin = cit->first;
				interval.end = cit->second;
				reg.channel_set.push_back(interval);
			}

			for(cit = backup_channel_sets.begin(); cit != backup_channel_sets.end(); ++cit )
			{
				log( Info, "DoubleLinkClient::subscribe,service:%s,backup_channel_set(%d,%d) register to proxy(%s,%d)",
						getServiceFlag().c_str(),cit->first,cit->second,it->first.c_str(),it->second);
				ChannelInterval interval;
				interval.begin = cit->first;
				interval.end = cit->second;
				reg.backup_channel_set.push_back(interval);
			}

			Sender send(ChannelsSubscribeRegister::uri,reg);
			dispatchByProxy(reg.to,send);
		}
	}

	void DoubleLinkClient::unsubscribe(const set<pair<uint32_t,uint32_t> >& cset, const set<pair<uint32_t,uint32_t> >& backup_cset)
	{
	}

	void DoubleLinkClient::registerOnConn(const string& proxy,uint32_t connid)
	{
		log( Info, "DoubleLinkClient::registerOnConn,subscribe to proxy(%s),connid:%d ",
				proxy.c_str(), cid );
		ChannelsSubscribeRegister reg;
		reg.service_flag = getServiceFlag();
		reg.to = proxy;

		set<pair<uint32_t,uint32_t> >::iterator cit;
		for(cit = channel_sets.begin(); cit != channel_sets.end(); ++cit )
		{
			ChannelInterval interval;
			interval.begin = cit->first;
			interval.end = cit->second;
			reg.channel_set.push_back(interval);
		}

		for(cit = backup_channel_sets.begin(); cit != backup_channel_sets.end(); ++cit )
		{ 
			ChannelInterval interval;
			interval.begin = cit->first;
			interval.end = cit->second;
			reg.backup_channel_set.push_back(interval);
		}

		Sender send(ChannelsSubscribeRegister::uri,reg);
		send.endPack();
		MultiConnManagerImp::dispatchById(cid,send);
	}

	void DoubleLinkClient::onRequestRoute(PRequestRouter* r, core::IConn* c)
	{
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onRequestRoute,no net observer" );
		Request req(r->load.c_str(),r->load.size());
		req.head();
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleRequest(proxy, r->channel_id, r->subchannel_id, r->uid, r->suid, r->platform, r->ruri,  req);
	}

	void DoubleLinkClient::onExtResponseRoute(PExtResponseRouter* r, core::IConn* c)
	{
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onExtResponseRoute,no net observer" );
		//Request req(r->load.c_str(),r->load.size());
		//req.head();

		sox::Unpack up(r->load.c_str(),r->load.size());
		log( Info, "DoublelinkClient::onExtResponseRoute,channel:%u,uid:%u,uri:%u",r->channel_id,r->uid,r->ruri);
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleRequest(proxy, r->channel_id, r->subchannel_id, r->uid, r->ruri,  up);
	}

	void DoubleLinkClient::onPingResponse(PPingResp* p, core::IConn* c)
	{
		//log( Info, "DoubleLinkClient::onPingResponse, conn:%d ping response", c->getConnId() );
		conn_timestamp_map[c->getConnId()] = time(NULL);
	}

	void DoubleLinkClient::onQueryByChannelReq(PQueryByChannelReq* r, core::IConn* c)
	{
		log( Info, "DoubleLinkClient::onQueryByChannelReq,outside connid:%u,load size:%Zu",r->outside_connid,r->load.size() );
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onQueryByChannelReq,no net observer" );
		//Request req(r->load.c_str(),r->load.size());
		//log( Info, "11111111111111111");
		//req.head();
		//log( Info, "22222222222222");
		sox::Unpack up(r->load.c_str(),r->load.size());
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		//log( Info, "3333333333333");
		net_observer->handleQueryByChannelRequest(proxy, r->channel_id, r->subchannel_id, r->uid, r->ruri, r->outside_connid, up);
		//log( Info, "4444444444");
	}

	void DoubleLinkClient::onQueryByUidReq(PQueryByUidReq* r, core::IConn* c)
	{
		log( Info, "DoubleLinkClient::onQueryByUidReq,outside connid:%u",r->outside_connid );
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onQueryByUidReq,no net observer" );
		//Request req(r->load.c_str(),r->load.size());
		//req.head();
		sox::Unpack up(r->load.c_str(),r->load.size());
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleQueryByUidRequest(proxy, r->uid, r->ruri, r->outside_connid, up);
	}

	void DoubleLinkClient::onEndSessionReq(PEndSessionReq* r, core::IConn* c)
	{
		log( Info, "DoubleLinkClient::onEndSessionReq,outside session_id:%s",r->session.c_str() );
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onEndSessionReq,no net observer" );

		//sox::Unpack up(r->load.c_str(),r->load.size());
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleEndSessionRequest(proxy, r->session,r->lesson_id);
	}

	void DoubleLinkClient::onThriftNotice(PThriftNoticeRouter* r, core::IConn* c)
	{
		log( Info, "DoubleLinkClient::onThriftNotice,load size:%Zu",r->load.size() );
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onThriftNotice,no net observer" );
		sox::Unpack up(r->load.c_str(),r->load.size());
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleQueryByChannelRequest(proxy, r->channel_id, r->subchannel_id, 0, r->ruri, 0, up);
	}


	void DoubleLinkClient::onEndSessionForce(PEndSessionForceRouter* r, core::IConn* c)
	{
		log( Info, "DoubleLinkClient::onEndSessionForce,load size:%Zu",r->load.size() );
		if( net_observer == NULL )
			log( Error,"DoubleLinkClient::onEndSessionForce,no net observer" );
		sox::Unpack up(r->load.c_str(),r->load.size());
		PROXY_INFO_T proxy = getProxyInfoFromConnId(c->getConnId());
		net_observer->handleQueryByChannelRequest(proxy, r->channel_id, r->subchannel_id, 0, r->ruri, 0, up);
	}


} 
