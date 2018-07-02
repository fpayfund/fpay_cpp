#include "DoubleLinkRouter.h"
#include "common/core/form.h"
#include "core/sox/logger.h"
#include "common/core/ibase.h"
#include "ServiceProxyLayer.h"
#include "BroadcasterClient2.h"
#include "packet.h"
#include "doublelinkdefine.h"

//#include "service_type.h"

using namespace core;
using namespace sdaemon;

using ::server::service::ServiceProxyLayer;
using ::server::service::BroadcasterClient2;
using ::server::service::UnicastClient;

namespace doublelink {

	//const std::string dispatch_route_name_ = "edu_extend_server";
	const uint32_t TIMER_SHOW_STAT_INFO = 1000 * 5; 
	const uint32_t TIMER_CHECK_PING_TIMEOUT = 1000 * 1;
	const uint32_t TIMER_SHOW_REGISTER = 1000* 1; 
	const uint32_t TIMER_CLEAN_UIDINFO = 1000* 60;
	const uint32_t TIMER_CLEAN_CHANNELINFO = 1000* 15;
	const uint32_t TIMER_CLEAN_SESSIONINFO = 1000* 30;
	const uint32_t CONN_TIMEOUT = 12;

	BEGIN_FORM_MAP(DoubleLinkRouter)
		ON_LINK(ChannelsSubscribeRegister, &DoubleLinkRouter::onRegister)
		ON_LINK(ChannelsUnSubscribeRegister, &DoubleLinkRouter::onUnRegister)
		ON_LINK(PResponseRouter, &DoubleLinkRouter::onResponse)
		ON_LINK(PMulticastRouter, &DoubleLinkRouter::onMulticast)
		ON_LINK(PBroadcastByChannelRouter, &DoubleLinkRouter::onBroadcastByChannel)
		ON_LINK(PBroadcastBySubChannelRouter, &DoubleLinkRouter::onBroadcastBySubChannel)
		ON_LINK(PPingReq, &DoubleLinkRouter::onPing)
		ON_LINK(PULServiceMsgHash10, &DoubleLinkRouter::onPULServiceMsgHash2)

		ON_LINK(PExtRequestRouter, &DoubleLinkRouter::onExtRequest)
		ON_LINK(PExtResponseRouter, &DoubleLinkRouter::onExtResponse)
		ON_LINK(PQueryByChannelReq,&DoubleLinkRouter::onQueryByChannelRequest)
		ON_LINK(PQueryByChannelResp,&DoubleLinkRouter::onQueryByChannelResponse)

		ON_LINK(PQueryByUidReq,&DoubleLinkRouter::onQueryByUidRequest)
		ON_LINK(PQueryByUidResp,&DoubleLinkRouter::onQueryByUidResponse)

		ON_LINK(PQueryConnIdReq,&DoubleLinkRouter::onQueryConnidsRequest)
		ON_LINK(PUserInfoReportRouter,&DoubleLinkRouter::onUserInfoReport)


		ON_LINK(PGetUserInfoReq,&DoubleLinkRouter::onGetUserInfoReq)
		ON_LINK(PGetSessionInfoReq,&DoubleLinkRouter::onGetSessionInfoReq)

		ON_LINK(PEndSessionReq,&DoubleLinkRouter::onEndSessionReq)
		ON_LINK(PThriftNoticeRouter,&DoubleLinkRouter::onThriftNotice)
		ON_LINK(PEndSessionForceRouter,&DoubleLinkRouter::onEndSessionForce)

		END_FORM_MAP()


		DoubleLinkRouter::DoubleLinkRouter(ServiceProxyLayer& spl,BroadcasterClient2& bc,UnicastClient& uc):
			sid(0),
			timer_stat(this),
			timer_check_ping(this),
			timer_show_register(this),
			timer_clean_uidinfo(this),
			timer_clean_channelinfo(this),
			timer_clean_sessioninfo(this),
			response_count(0),
			channel_broadcast_count(0),
			subchannel_broadcast_count(0),
			multicast_count(0)
	{
		proxy_layer_ = &spl;
		broadcast_cli_ = &bc;
		unicast_cli_ = &uc;
		timer_stat.start(TIMER_SHOW_STAT_INFO);
		timer_check_ping.start(TIMER_CHECK_PING_TIMEOUT);
		timer_show_register.start(TIMER_SHOW_REGISTER);

		timer_clean_uidinfo.start(TIMER_CLEAN_UIDINFO);
		timer_clean_channelinfo.start(TIMER_CLEAN_CHANNELINFO);
		timer_clean_sessioninfo.start(TIMER_CLEAN_SESSIONINFO);

		//log(Debug, "DoubleLinkRouter::ctor(dispatch_route_name:%s)", dispatch_route_name_.c_str());
	}

	DoubleLinkRouter::~DoubleLinkRouter()
	{
	}

	void DoubleLinkRouter::eraseConnect(IConn *conn)
	{
		uint32_t cid = conn->getConnId();
		cid2responses.erase(cid);
		string service_flag = cid2service[cid];
		if( service_flag == "" )/* not business conn*/
		{
			pending_query_conns_.erase(conn->getConnId());
			log( Info, "DoubleLinkRouter::eraseConnect,outside_connid:%u",conn->getConnId());
			MultiConnManagerImp::eraseConnect(conn);
			return;
		}
		map<string,service_info_t>::iterator scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );
		vector<conn_info_t>::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); )
		{
			if( cit->cid == cid )
			{
				cit = scit->second.conns.erase(cit);
				log( Info, "DoubleLinkRouter::eraseConnect, service:%s, remove conn:%d",service_flag.c_str(), cid );
			}
			else
			{
				cit++;
			}
		}
		if( scit->second.conns.empty() )
		{
			uint64_t sid = scit->second.sid;
			service2conns.erase(scit);
			sid2ServiceFlag.erase(sid);
			channel2Sid.erase(sid);
			backupChannel2Sid.erase(sid); 
			log( Warn, "DoubleLinkRouter::eraseConnect, service(%s,%ld) is remove", service_flag.c_str(), sid);
		}
		cid2service.erase(cid);

		MultiConnManagerImp::eraseConnect(conn);
	}

	bool DoubleLinkRouter::findMasterServiceByChannel( uint32_t channel_id, string& service_flag )
	{
		set<uint64_t> sids = channel2Sid.getTargetIdBySid(channel_id);
		if( !sids.empty() )
		{
			set<uint64_t>::iterator it = sids.begin();
			service_flag = sid2ServiceFlag[*it];
			return true;
		}
		return false;
	}

	bool DoubleLinkRouter::findSlaveServiceByChannel( uint32_t channel_id, string& service_flag )
	{
		set<uint64_t> sids = backupChannel2Sid.getTargetIdBySid(channel_id);
		if( !sids.empty() )
		{
			set<uint64_t>::iterator it = sids.begin();
			service_flag = sid2ServiceFlag[*it];
			log( Info, "find slave, sid(%ld) channel:%u, service:%s", *it, channel_id, service_flag.c_str() );
			return true;
		}
		return false;
	}


	uint64_t DoubleLinkRouter::findMasterSidByChannel( uint32_t channel_id )
	{
		set<uint64_t> sids = channel2Sid.getTargetIdBySid(channel_id);
		if( !sids.empty() )
		{
			set<uint64_t>::iterator it = sids.begin();
			return *it;
		}
		return 0;
	}

	uint64_t DoubleLinkRouter::findSlaveSidByChannel( uint32_t channel_id )
	{
		set<uint64_t> sids = backupChannel2Sid.getTargetIdBySid(channel_id);
		if( !sids.empty() )
		{
			set<uint64_t>::iterator it = sids.begin();
			return *it;
		}
		return 0;
	}

	uint64_t DoubleLinkRouter::findSidByConnId( uint32_t cid )
	{ 
		map<string/*service flag*/,service_info_t>::iterator it = service2conns.find(cid2service[cid]);
		assert(it != service2conns.end());
		return it->second.sid;       
	}

	bool DoubleLinkRouter::findServiceByChannel( uint32_t channel_id, string& service_flag )
	{    
		return findMasterServiceByChannel(channel_id,service_flag) ? true : findSlaveServiceByChannel(channel_id,service_flag);
	}

	void DoubleLinkRouter::onRegister(ChannelsSubscribeRegister *reg, IConn* c)
	{
		conn_info_t cinfo;
		cinfo.cid = c->getConnId();
		cinfo.proxy = reg->to; 
		map<string,service_info_t>::iterator scit;
		scit = service2conns.find(reg->service_flag);
		if( scit == service2conns.end() )
		{
			sid++;
			sid2ServiceFlag[sid] = reg->service_flag;

			service_info_t serv_info;
			serv_info.service_flag = reg->service_flag;
			serv_info.sid = sid;
			serv_info.channel_set = reg->channel_set;
			serv_info.backup_channel_set = reg->backup_channel_set;
			serv_info.conns.push_back(cinfo);
			service2conns[reg->service_flag] = serv_info;

			vector<ChannelInterval>::iterator ciit;
			for( ciit = reg->channel_set.begin(); ciit != reg->channel_set.end(); ++ciit )
			{
				channel2Sid.insert(ciit->begin,ciit->end,sid);
				log( Info, "service:%s,channel set(%u-%u) register,sid:%ld\n",reg->service_flag.c_str(),ciit->begin,ciit->end,sid);
			}
			//backup channel set
			for( ciit = reg->backup_channel_set.begin(); ciit != reg->backup_channel_set.end(); ++ciit )
			{
				backupChannel2Sid.insert(ciit->begin,ciit->end,sid);
				log( Info, "service:%s,back channel set(%u-%u) register,sid:%ld\n",reg->service_flag.c_str(),ciit->begin,ciit->end,sid);
			}
		}
		else
		{
			scit->second.conns.push_back(cinfo);
		}
		cid2service[c->getConnId()] = reg->service_flag;
		cid2responses[c->getConnId()] = 0;
	}

	void DoubleLinkRouter::onUnRegister(ChannelsUnSubscribeRegister *unreg, IConn* c)
	{

	}

	void DoubleLinkRouter::onResponse(PResponseRouter* r,core::IConn* c)
	{

		log( Info, "recv response,channel:%u,uid:%u,suid:%lu,platform:%u,uri:%u",r->channel_id,r->uid,r->suid,r->platform,r->ruri);
		uint64_t sid = findMasterSidByChannel( r->channel_id );
		if( sid != 0 && sid != findSidByConnId(c->getConnId()) )
		{
			log( Info, "DoubleLinkRouter::onResponse drop, beacuse master service(%lu) is up,channel:%u, uid:%u,suid:%lu,platform:%u,uri:%u", sid, r->channel_id, r->uid,r->suid,r->platform,r->ruri);
			return;
		}
		cid2responses[c->getConnId()]++; 
		Sender send;
		send.setUri(r->ruri);
		send.marshall(r->load.c_str(),r->load.size());
		send.endPack();
		//proxy_layer_->sendMsgToClient(r->uid, send.header(), send.headerSize() + send.bodySize());
		string msg;
		msg.assign(send.header(),send.headerSize()+send.bodySize());
		proxy_layer_->sendMsgToClient(r->suid,msg);

		response_count++;
	}

	/*void DoubleLinkRouter::onMulticast(PMulticastRouter* r,core::IConn* c)
	  {
	  uint64_t sid = findMasterSidByChannel( r->channel_id );
	  if( sid != 0 && sid != findSidByConnId(c->getConnId()) )
	  {
	  log( Info, "DoubleLinkRouter::onMulticast drop, beacuse master service(%ld) is up,channel:%d", sid, r->channel_id);
	  return;
	  }


	  Sender send;
	  send.setUri(r->ruri);
	  send.marshall(r->load.c_str(),r->load.size());
	  send.endPack();    
	//broadcast_cli_->multicastByUidSet(r->channel_id, r->uids, r->from_uid, send.header(), send.headerSize() + send.bodySize());

	string obj;
	obj.assign(send.header(),send.headerSize() + send.bodySize()); 
	unicast_cli_->multicastToClient(obj,r->uids,r->channel_id,0);
	multicast_count++;

	//log( Info, "DoubleLinkRouter::onMult, response,uri:%d,channel_id:%u, from uid:%u,multicast_count:%lu", r->ruri, r->channel_id, r->from_uid, multicast_count);
	}*/ //by fanzhihui 2015-3-19

	//add by fanzhihui 2015-3-19
	void DoubleLinkRouter::onMulticast(PMulticastRouter* r,core::IConn* c)
	{
		uint64_t sid = findMasterSidByChannel( r->channel_id );
		if( sid != 0 && sid != findSidByConnId(c->getConnId()) )
		{
			log( Info, "DoubleLinkRouter::onMulticast drop, beacuse master service(%ld) is up,channel:%d", sid, r->channel_id);
			return;
		}


		Sender send;
		send.setUri(r->ruri);
		send.marshall(r->load.c_str(),r->load.size());
		send.endPack();
		//broadcast_cli_->multicastByUidSet(r->channel_id, r->uids, r->from_uid, send.header(), send.headerSize() + send.bodySize());

		string obj;
		obj.assign(send.header(),send.headerSize() + send.bodySize());
		bool ret = unicast_cli_->multicastToClient(obj,r->uids);
		
		multicast_count++;
	    set<uint32_t>::iterator to_it;
		for(to_it = r->uids.begin(); to_it != r->uids.end(); ++to_it) {
			log( Info, "DoubleLinkRouter::onMult,ret:%s,uri:%d,channel_id:%u, from uid:%u,to uid:%u", ret? "true":"false", r->ruri, r->channel_id, r->from_uid, *to_it);
		}
	}

	void DoubleLinkRouter::onBroadcastByChannel(PBroadcastByChannelRouter* r,core::IConn* c)
	{
		uint64_t sid = findMasterSidByChannel( r->channel_id );
		if( sid != 0 && sid != findSidByConnId(c->getConnId()) )
		{
			log( Info, "DoubleLinkRouter::onBroadcastByChannel drop, beacuse master service(%ld) is up,channel:%d", sid, r->channel_id);
			return;
		}
		//log( Info, "DoubleLinkRouter::onBroadcastByChannel,response,uri:%d",r->ruri );
		Sender send;
		send.setUri(r->ruri);
		send.marshall(r->load.c_str(),r->load.size());
		send.endPack();
		broadcast_cli_->broadcastByTopSid(r->channel_id, send.header(), send.headerSize() + send.bodySize());
		channel_broadcast_count++;
	}

	void DoubleLinkRouter::onBroadcastBySubChannel(PBroadcastBySubChannelRouter* r,core::IConn* c)
	{
		uint64_t sid = findMasterSidByChannel( r->channel_id );
		if( sid != 0 && sid != findSidByConnId(c->getConnId()) )
		{        
			log( Info, "DoubleLinkRouter::onBroadcastBySubChannel drop, beacuse master service(%ld) is up,channel:%d", sid, r->channel_id);
			return;
		}
		log( Info, "subsid broadcast (%u,%u,%u)", r->channel_id, r->subchannel_id, r->ruri );
		Sender send;
		send.setUri(r->ruri);
		send.marshall(r->load.c_str(),r->load.size());
		send.endPack();
		broadcast_cli_->broadcastBySubSid(r->channel_id, r->subchannel_id, send.header(), send.headerSize() + send.bodySize());
		subchannel_broadcast_count++;
	}

	void DoubleLinkRouter::onPing(PPingReq* p, core::IConn* c)
	{
		//log( Info, "DoubleLinkRouter::onPing.");
		chanagePingTimestamp( c->getConnId() );
		pingResponse(p->service_flag,c->getConnId());
	}

	void DoubleLinkRouter::chanagePingTimestamp(uint32_t cid)
	{
		time_t now = time(NULL);
		map<string,service_info_t>::iterator scit;
		scit = service2conns.find( cid2service[cid] );
		assert( scit != service2conns.end() );
		vector<conn_info_t>::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( cit->cid == cid )
			{
				//log( Info, "DoubleLinkRouter::chanagePingTimestamp.curtime:%u,last pint time:%u", (uint32_t)now, (uint32_t)cit->last_ping_time);
				cit->last_ping_time = now;
				return;
			}
		}
	}

	void DoubleLinkRouter::pingResponse( const string& service, uint32_t cid)
	{
		PPingResp  pr;
		pr.service_flag = service;

		Sender send;
		send.marshall(PPingResp::uri,pr);
		send.endPack();
		if( !dispatchById( cid, send ) )
		{
			log( Info, "DoubleLinkRouter::pingResponse,response ping to service:%s faild,conn:%d",service.c_str(), cid );
		}
	}

	void DoubleLinkRouter::dispatchRequest(uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, const string& load)
	{

		log( Info, "recv request,channel:%u, uid:%u, suid:%lu,platform:%u uri:%u,",channel_id, uid, suid,platform,uri );
		string service_flag;
		if( findServiceByChannel(channel_id, service_flag ) == false )
		{
			log( Error, "DoubleLinkRouter::dispatchRequest,channel:%u, uid:%u, suid:%lu,platform:%u, uri:%u,not find service",
					channel_id, uid, suid, platform, uri );
			return; 
		}

		PRequestRouter route;
		route.channel_id = channel_id;
		route.uid = uid;
		route.suid = suid;
		route.platform = platform;

		route.ruri = uri;
		route.from = "";
		route.to = "";
		route.load = load;
		Sender send;
		send.marshall(PRequestRouter::uri,route);
		send.endPack();

		map<string,service_info_t>::iterator scit;
		/*for( scit = service2conns.begin(); scit != service2conns.end(); ++scit )
		  {
		//fprintf(stderr,"dispatch, %s,sid:%ld, connsize:%Zu\n",scit->first.c_str(), scit->second.sid, scit->second.conns.size());
		}*/
		scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );

		vector<conn_info_t> ::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( dispatchById( cit->cid, send ) == true )
			{
				cit->req_count++;
				return;
			}
		}        
		log( Error, "DoubleLinkRouter::dispatchRequest,channel:%u, uid:%u, suid:%lu, platform:%u,uri:%u, send error",
				channel_id, uid, suid, platform, uri ); 
	}

	bool DoubleLinkRouter::showStatInfo()
	{
		uint32_t service_serial = 0;
		map<string,service_info_t>::iterator scit;
		for( scit = service2conns.begin(); scit != service2conns.end(); ++scit )
		{
			log( Info, "doublelink stat========================================================" );
			log( Info, "doublelink stat=>servcie(%d):%s", service_serial,scit->first.c_str() );
			log( Info, "doublelink stat===>channel set:" );
			vector<ChannelInterval>::iterator chit;
			for( chit = scit->second.channel_set.begin(); chit != scit->second.channel_set.end(); ++chit )
			{
				log( Info,"doublelink stat======>%d-%d",chit->begin,chit->end );
			}
			log( Info, "doublelink stat===>backup channel set:" );
			for( chit = scit->second.backup_channel_set.begin(); chit != scit->second.backup_channel_set.end(); ++chit )
			{
				log( Info,"doublelink stat======>%d-%d",chit->begin,chit->end );
			}
			log( Info, "doublelink stat===>conn info:" );
			vector<conn_info_t>::iterator cit;
			for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
			{
				log( Info,"doublelink stat======>conn:%d,proxyip:%s,request:%lld,response:%lld",cit->cid,
						get_ip_from_proxy_info(cit->proxy).c_str(),cit->req_count,cid2responses[cit->cid]);
			}
			service_serial++;
		}
		log(Info,"doublelink stat:response:%lld,multicast:%lld,channel broadcast:%lld,subchannel broadcast:%lld",
				response_count, multicast_count, channel_broadcast_count, subchannel_broadcast_count ); 
		return true;
	}

	bool DoubleLinkRouter::checkPingTimeout()
	{
		time_t now = time(NULL);
		set<uint32_t> bad_conns;
		map<string,service_info_t>::iterator scit;
		for( scit = service2conns.begin(); scit != service2conns.end(); ++scit )
		{
			vector<conn_info_t>::iterator cit;
			for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
			{
				if( now - cit->last_ping_time > CONN_TIMEOUT )
				{
					log( Error, "servie:%s,conn:%u ping timeout", scit->first.c_str(), cit->cid );
					bad_conns.insert( cit->cid );
				}
			}
		}
		set<uint32_t>::iterator bad_it;
		for( bad_it = bad_conns.begin(); bad_it != bad_conns.end(); ++bad_it )
		{
			eraseConnectById(*bad_it);
		}
		return true;
	}

	bool DoubleLinkRouter::showRegister()
	{  
		RANGE_MAP::iterator it;
		log( Info,"-------------------------------------------------------------------");
		for( it = channel2Sid.m_rangeMap.begin(); it != channel2Sid.m_rangeMap.end(); ++it )
		{
			log( Info, "channel set(%u-%u) sid:%ld", it->first.from, it->first.to, it->second.id );
		}
		for( it = backupChannel2Sid.m_rangeMap.begin(); it != backupChannel2Sid.m_rangeMap.end(); ++it )
		{
			log( Info, "backup channel set(%u-%u) sid:%ld", it->first.from, it->first.to, it->second.id );
		}
		log( Info,"-------------------------------------------------------------------");
		return true;
	}


	void DoubleLinkRouter::onPULServiceMsgHash2(PULServiceMsgHash10* r,core::IConn* c)
	{
		//dispatchRequest(r->sid, r->uid, 0, r->msg);
	}

	void DoubleLinkRouter::onExtRequest(PExtRequestRouter* r,core::IConn* c)
	{   
		Request req(r->load.c_str(),r->load.size());
		req.head();

		log( Info,"recve ext request,%u,%u,uid:%u,uri:%u,loadsize:%u,uri:%u,msg_seq:%u,need return:%s,dispat router name:%s",r->channel_id,r->subchannel_id,r->uid,r->ruri,r->load.size(),req.getUri(),r->msg_seq,r->need_return == 1 ? "true":"false",r->dispatch_router_name.c_str() );

		//sDispatcher->dispatchToServerRandom(dispatch_route_name_, PExtRequestRouter::uri, *r);
		sDispatcher->DispatchToServerRoundRobin(r->dispatch_router_name, PExtRequestRouter::uri, *r);
	}

	void DoubleLinkRouter::onExtResponse(PExtResponseRouter* r, core::IConn* c)
	{
		log( Info,"recve ext response,%u,%u,uid:%u,uri:%u,msg_seq:%u",r->channel_id,r->subchannel_id,r->uid,r->ruri,r->msg_seq );
		uint32_t channel_id = r->channel_id;
		string service_flag;
		if( findServiceByChannel(channel_id, service_flag ) == false )
		{
			log( Error, "DoubleLinkRouter::onExtResponse,channel:%d,uri:%d,not find service",
					channel_id,r->ruri );
			return;
		}
		Sender send;
		send.marshall(PExtResponseRouter::uri,*r);
		send.endPack();

		map<string,service_info_t>::iterator scit;
		scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );

		vector<conn_info_t> ::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( dispatchById( cit->cid, send ) == true )
			{      
				return;
			}
		}
		log( Error, "DoubleLinkRouter::onExtResponse,channel:%d,uri:%d, send error",
				channel_id,r->ruri );

	}

	void DoubleLinkRouter::onQueryByChannelRequest(PQueryByChannelReq* r, core::IConn* c)
	{
		uint32_t channel_id = r->channel_id;
		string service_flag;
		if( findServiceByChannel(channel_id, service_flag ) == false )
		{
			log( Error, "DoubleLinkRouter::onQueryByChannelRequest,channel:%d,uri:%d,not find service",
					channel_id,r->ruri );
			eraseConnect(c);
			return;
		}
		r->outside_connid = c->getConnId();  
		Sender send;
		send.marshall(PQueryByChannelReq::uri,*r);
		send.endPack();

		map<string,service_info_t>::iterator scit;
		scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );

		log( Info, "oubleLinkRouter::onQueryByChannelRequest,channel:%u,outside_connid:%u,r->ruri:%u,service:%s",channel_id,r->outside_connid,r->ruri,service_flag.c_str());
		vector<conn_info_t> ::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( dispatchById( cit->cid, send ) == true )
			{
				cit->req_count++;
				pending_query_conns_.insert(c->getConnId());
				return;
			}
		}
		log( Error, "DoubleLinkRouter::onQueryByChannelRequest,channel:%d,uri:%d, send error",
				channel_id,r->ruri );
	}

	void DoubleLinkRouter::onQueryByChannelResponse(PQueryByChannelResp* r, core::IConn* c)
	{
		log( Info, "begin DoubleLinkRouter::onQueryByChannelResponse,r->outside_connid:%u",r->outside_connid);
		if( pending_query_conns_.find(r->outside_connid) != pending_query_conns_.end())
		{
			Sender send;
			send.marshall(PQueryByChannelResp::uri,*r);
			send.endPack();
			if( dispatchById( r->outside_connid, send ) == true )
			{
				log( Info, "success DoubleLinkRouter::onQueryByChannelResponse,r->outside_connid:%u",r->outside_connid);
				pending_query_conns_.erase(r->outside_connid);
				return;
			}
		}
	}


	void DoubleLinkRouter::onQueryConnidsRequest(PQueryConnIdReq* r, core::IConn* c)
	{
		log( Info, "oubleLinkRouter::onQueryConnidsRequest");
		PQueryConnIdResp resp;
		RANGE_MAP::iterator it;
		for( it = channel2Sid.m_rangeMap.begin(); it != channel2Sid.m_rangeMap.end(); ++it )
		{ 
			string service_flag = sid2ServiceFlag[it->second.id]; 
			map<string,service_info_t>::iterator scit;
			scit = service2conns.find( service_flag );
			assert( scit != service2conns.end() );
			vector<conn_info_t> ::iterator cit;
			for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
			{
				log( Info,"oubleLinkRouter::onQueryConnidsRequest,service_flag:%s,connid:%u",service_flag.c_str(),cit->cid );
				resp.connids.push_back(cit->cid);
				break;
			}

		}

		log( Info,"oubleLinkRouter::onQueryConnidsRequest,connid size:%Zu",resp.connids.size());


		Sender send;
		send.marshall(PQueryConnIdResp::uri,resp);
		send.endPack();
		if (dispatchById( c->getConnId(), send ) == false)
		{
			log( Info, "oubleLinkRouter::onQueryConnidsRequest,send resp err");
		}

	}

	void DoubleLinkRouter::onQueryByUidRequest(PQueryByUidReq* r, core::IConn* c)
	{
		log( Info, "oubleLinkRouter::onQueryByUidRequest,uid:%u",r->uid);
		r->outside_connid = c->getConnId();
		Sender send;
		send.marshall(PQueryByUidReq::uri,*r);
		send.endPack();

		if( dispatchById( r->req_connid, send ) == true )
		{
			pending_query_conns_.insert(c->getConnId());
			return; 
		}
		//resp err
		PQueryByUidResp resp;

		Sender rsp_send;
		rsp_send.marshall(PQueryByUidResp::uri,resp);
		rsp_send.endPack();
		dispatchById( c->getConnId(), rsp_send );
	}

	void DoubleLinkRouter::onQueryByUidResponse(PQueryByUidResp* r, core::IConn* c)
	{
		if( pending_query_conns_.find(r->outside_connid) != pending_query_conns_.end() )
		{
			Sender send;
			send.marshall(PQueryByUidResp::uri,*r);
			send.endPack();
			if( dispatchById( r->outside_connid, send ) == true )
			{
				pending_query_conns_.erase(r->outside_connid);
				return;
			}
		}
	}

	void DoubleLinkRouter::onUserInfoReport(PUserInfoReportRouter* r,core::IConn* c)
	{
		//std::vector<UserInfo>::iterator it;
		std::map<uint32_t/*channel_id*/,channel_info_t >::iterator mit;

		if( r->op == 0 ) 
		{
			log( Info,"DoubleLinkRouter::onUserInfoReport,channel:%u,add uid:%u,json:%s",r->user_info.channel_id,r->user_info.uid,r->user_info.json.c_str());
			user_info_t user_info;
			user_info.uid = r->user_info.uid;
			user_info.channel_id = r->user_info.channel_id;
			user_info.subchannel_id = r->user_info.subchannel_id;
			user_info.json = r->user_info.json;
			user_info.last_heartbeat = time(NULL);
			uid2userinfo_[user_info.uid] = user_info;

			string session;
			uint8_t operate = 0;       
			if( DoubleLink_PraseSessionInfo(user_info.json,session,operate) )
			{ 
				log( Info, "DoubleLinkRouter::onUserInfoReport,session:%s,operate:%u",session.c_str(),operate); 
				std::map<string/*session*/,session_info_t>::iterator sess_it;
				if( (sess_it = session2sessioninfo_.find(session)) != session2sessioninfo_.end() )
				{
					sess_it->second.channel_id = r->user_info.channel_id;
					sess_it->second.subchannel_id = r->user_info.subchannel_id;
					sess_it->second.uids.insert(r->user_info.uid);
					sess_it->second.last_heartbeat = time(NULL);
				}
				else
				{
					session_info_t sess_info;
					sess_info.session = session;
					sess_info.channel_id = r->user_info.channel_id;
					sess_info.subchannel_id = r->user_info.subchannel_id;
					sess_info.uids.insert(r->user_info.uid);
					sess_info.last_heartbeat = time(NULL);
					session2sessioninfo_[session] = sess_info;
				}
			}        

			mit = channel_2_uids_.find(user_info.channel_id);
			if( mit == channel_2_uids_.end() )
			{  
				channel_info_t chan_info;
				chan_info.uids.insert(user_info.uid);
				chan_info.channel_id = user_info.channel_id;
				chan_info.last_heartbeat = time(NULL);
				channel_2_uids_[user_info.channel_id] = chan_info;
			}
			else
			{
				mit->second.uids.insert(user_info.uid);  
				mit->second.last_heartbeat = time(NULL); 
			}
		}
		else {
			log( Info,"DoubleLinkRouter::onUserInfoReport,channel:%u,remove uid:%u",r->user_info.channel_id,r->user_info.uid);
			uid2userinfo_.erase(r->user_info.uid);
			mit = channel_2_uids_.find(r->user_info.channel_id);
			if( mit != channel_2_uids_.end() )
			{  
				mit->second.uids.erase(r->user_info.uid);
			}

			string session;
			uint8_t operate = 0;
			if( DoubleLink_PraseSessionInfo(r->user_info.json,session,operate) )
			{
				if( operate == 1 )
				{
					log( Info, "DoubleLinkRouter::onUserInfoReport,remove session:%s,channel:%u",session.c_str(),r->user_info.channel_id);
					session2sessioninfo_.erase(session);
				}
				else
				{
					std::map<string/*session*/,session_info_t>::iterator sess_it;
					if( (sess_it = session2sessioninfo_.find(session)) != session2sessioninfo_.end() )
					{
						sess_it->second.uids.erase(r->user_info.uid);
					}
					else
					{
						log( Error, " uid:%u quit,but not fond session info for session:%s",r->user_info.uid,session.c_str());
					}
				}
			}      
		}

		log( Info, "DoubleLinkRouter::onUserInfoReport,connid:%u,user count:%Zu,channel count:%Zu,session count:%Zu",c->getConnId(),uid2userinfo_.size(),channel_2_uids_.size(),session2sessioninfo_.size());
	}

	void DoubleLinkRouter::onGetUserInfoReq(PGetUserInfoReq* r, core::IConn* c)
	{
		log( Info, "oubleLinkRouter::onGetUserInfoReq,uid:%u",r->uid);

		std::map<uint32_t/*uid*/,user_info_t>::iterator u2c_it;
		std::map<uint32_t/*channel_id*/,channel_info_t >::iterator c2u_it;    
		//resp err
		PGetUserInfoResp resp;
		u2c_it = uid2userinfo_.find(r->uid);
		if( u2c_it != uid2userinfo_.end() )
		{
			resp.channel_id = u2c_it->second.channel_id;
			resp.subchannel_id = u2c_it->second.subchannel_id;
			c2u_it = channel_2_uids_.find(resp.channel_id);
			if( c2u_it != channel_2_uids_.end() )
			{
				std::set<uint32_t>::iterator uit;
				for( uit = c2u_it->second.uids.begin(); uit != c2u_it->second.uids.end(); ++uit )
				{
					resp.uids.push_back(*uit);
				}
			}
		}

		Sender rsp_send;
		rsp_send.marshall(PGetUserInfoResp::uri,resp);
		rsp_send.endPack();
		dispatchById( c->getConnId(), rsp_send );
	}


	void DoubleLinkRouter::onGetSessionInfoReq(PGetSessionInfoReq* r, core::IConn* c)
	{
		log( Info, "oubleLinkRouter::onGetSessionInfoReq,session:%s",r->session.c_str());

		std::map<string/*session*/,session_info_t>::iterator sess_it;
		//resp err
		PGetSessionInfoResp resp;
		sess_it = session2sessioninfo_.find(r->session);
		if( sess_it != session2sessioninfo_.end() )
		{
			resp.channel_id = sess_it->second.channel_id;
			resp.subchannel_id = sess_it->second.subchannel_id;
			std::set<uint32_t>::iterator uit;        
			for( uit = sess_it->second.uids.begin(); uit != sess_it->second.uids.end(); ++uit )
			{
				resp.uids.push_back(*uit);
			}
		}

		Sender rsp_send;
		rsp_send.marshall(PGetSessionInfoResp::uri,resp);
		rsp_send.endPack();
		dispatchById( c->getConnId(), rsp_send );
	}

	bool DoubleLinkRouter::cleanExpireUidInfo()
	{
		time_t now = time(NULL);
		std::map<uint32_t/*uid*/,user_info_t>::iterator it;
		for( it = uid2userinfo_.begin(); it != uid2userinfo_.end(); )
		{
			if( now - it->second.last_heartbeat > 6 * 11 )
			{
				log( Info, "uid:%u is expire, erase",it->first );       
				std::map<uint32_t/*channel*/,channel_info_t>::iterator mit;
				mit = channel_2_uids_.find(it->second.channel_id);
				if( mit != channel_2_uids_.end() )
				{
					mit->second.uids.erase(it->first);
				}
				uid2userinfo_.erase(it++);
			}
			else
			{
				++it;
			}
		}  
		return true;
	}

	bool DoubleLinkRouter::cleanExpireChannelInfo()
	{    
		time_t now = time(NULL);
		std::map<uint32_t/*channel*/,channel_info_t>::iterator it;
		for( it = channel_2_uids_.begin(); it != channel_2_uids_.end(); )
		{
			if( now - it->second.last_heartbeat > 6 * 11 )
			{
				log( Info, "channel:%u is expire, erase",it->first );
				channel_2_uids_.erase(it++);
			}
			else
			{
				++it;
			}
		} 
		return true;
	}

	bool DoubleLinkRouter::cleanExpireSessionInfo()
	{
		time_t now = time(NULL);
		std::map<string/*session*/,session_info_t>::iterator it;

		//for test
		for( it = session2sessioninfo_.begin(); it != session2sessioninfo_.end(); ++it)
		{
			log( Info, "debug by fzh session:%s,%s,%u,uid count:%Zu,now:%u,%u",it->first.c_str(),it->second.session.c_str(),it->second.channel_id,it->second.uids.size(),time(NULL),it->second.last_heartbeat);
		}

		for( it = session2sessioninfo_.begin(); it != session2sessioninfo_.end(); )
		{
			if( now - it->second.last_heartbeat > 6 * 11 )
			{
				log( Info, "session:%s is expire, erase",it->first.c_str() );
				session2sessioninfo_.erase(it++);
			}
			else
			{
				++it;
			}
		}
		return true;
	}

	void DoubleLinkRouter::onEndSessionReq(PEndSessionReq* r, core::IConn* c)
	{
		log( Info, "oubleLinkRouter::onEndSessionReq,session:%s",r->session.c_str());
		Sender send;
		send.marshall(PEndSessionReq::uri,*r);
		send.endPack();

		std::map<string/*session*/,session_info_t>::iterator it;
		it = session2sessioninfo_.find(r->session);
		if( it != session2sessioninfo_.end() )
		{
			string service_flag;
			if( findServiceByChannel(it->second.channel_id, service_flag ) == false )
			{
				log( Error, "DoubleLinkRouter::onEndSessionReq,channel:%u,session:%s,not find service",it->second.channel_id,r->session.c_str());
				eraseConnect(c);
				return;
			}
			map<string,service_info_t>::iterator scit;
			scit = service2conns.find( service_flag );
			assert( scit != service2conns.end() );
			vector<conn_info_t> ::iterator cit;
			for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
			{
				if( dispatchById( cit->cid, send ) == true )
				{
					log( Info, "DoubleLinkRouter::onEndSessionReq,channel:%u,session:%s,end session success",it->second.channel_id,r->session.c_str());             
					break;
				}
			}
		}
		log( Info, "DoubleLinkRouter::onEndSessionReq,channel:%u,session:%s,not found session",it->second.channel_id,r->session.c_str());
		PEndSessionResp resp;
		resp.resp_code = 0;
		Sender rsp_send;
		rsp_send.marshall(PEndSessionResp::uri,resp);
		rsp_send.endPack();
		dispatchById( c->getConnId(), rsp_send );
		return;

		/*RANGE_MAP::iterator it;
		  for( it = channel2Sid.m_rangeMap.begin(); it != channel2Sid.m_rangeMap.end(); ++it )
		  {
		  string service_flag = sid2ServiceFlag[it->second.id];
		  map<string,service_info_t>::iterator scit;
		  scit = service2conns.find( service_flag );
		  assert( scit != service2conns.end() );
		  vector<conn_info_t> ::iterator cit;
		  for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		  {
		  log( Info,"oubleLinkRouter::onEndSessionReq,service_flag:%s,connid:%u",service_flag.c_str(),cit->cid );
		  dispatchById( cit->cid, send );            
		  break;
		  }

		  }*/


	}

	void DoubleLinkRouter::onThriftNotice(PThriftNoticeRouter* r, core::IConn* c)
	{
		uint32_t channel_id = r->channel_id;
		string service_flag;
		if( findServiceByChannel(channel_id, service_flag ) == false )
		{
			log( Error, "DoubleLinkRouter::onThriftNotice,channel:%d,uri:%d,not find service",
					channel_id,r->ruri );
			eraseConnect(c);
			return;
		}
		Sender send;
		send.marshall(PThriftNoticeRouter::uri,*r);
		send.endPack();

		map<string,service_info_t>::iterator scit;
		scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );

		log( Info, "oubleLinkRouter::onThriftNotice,channel:%u,r->ruri:%u,service:%s",channel_id,r->ruri,service_flag.c_str());
		bool send_ok = false;
		vector<conn_info_t> ::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( dispatchById( cit->cid, send ) == true )
			{
				cit->req_count++;
				send_ok = true;
			}
		}
		PThriftNoticeRouterResp resp;
		resp.resp_code = 0;
		Sender rsp_send;
		rsp_send.marshall(PThriftNoticeRouterResp::uri,resp);
		rsp_send.endPack();
		if( send_ok == false ) {
			resp.resp_code = -1;
			log( Error, "DoubleLinkRouter::onThriftNotice,channel:%d,uri:%d, send error",
					channel_id,r->ruri );
		}
		dispatchById( c->getConnId(), rsp_send );
	}


	void DoubleLinkRouter::onEndSessionForce(PEndSessionForceRouter* r, core::IConn* c)
	{
		uint32_t channel_id = r->channel_id;
		string service_flag;
		if( findServiceByChannel(channel_id, service_flag ) == false )
		{
			log( Error, "DoubleLinkRouter::onEndSessionForce,channel:%d,uri:%d,not find service",
					channel_id,r->ruri );
			eraseConnect(c);
			return;
		}
		Sender send;
		send.marshall(PEndSessionForceRouter::uri,*r);
		send.endPack();

		map<string,service_info_t>::iterator scit;
		scit = service2conns.find( service_flag );
		assert( scit != service2conns.end() );

		log( Info, "oubleLinkRouter::onEndSessionForce,channel:%u,r->ruri:%u,service:%s",channel_id,r->ruri,service_flag.c_str());
		bool send_ok = false;
		vector<conn_info_t> ::iterator cit;
		for( cit = scit->second.conns.begin(); cit != scit->second.conns.end(); ++cit )
		{
			if( dispatchById( cit->cid, send ) == true )
			{
				cit->req_count++;
				send_ok = true;
			}
		}
		PEndSessionForceRouterResp resp;
		resp.resp_code = 0;
		Sender rsp_send;
		rsp_send.marshall(PEndSessionForceRouterResp::uri,resp);
		rsp_send.endPack();
		if( send_ok == false ) {
			resp.resp_code = -1;
			log( Error, "DoubleLinkRouter::onEndSessionForce,channel:%d,uri:%d, send error",
					channel_id,r->ruri );
		}
		dispatchById( c->getConnId(), rsp_send );
	}



}
