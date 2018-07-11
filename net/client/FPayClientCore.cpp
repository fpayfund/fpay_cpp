#include "FPayFPayFPayClientCoreCoreCore.h"
#include "common/core/form.h"
#include "core/sox/sockethelper.h"
#include "core/sox/logger.h"
#include "core/sox/udpsock.h"
#include "core/corelib/AbstractConn.h"
#include "core/corelib/InnerConn.h"
#include "doublelink_protocol.h"

using namespace core;
using namespace sox;
using namespace sdaemon;
using namespace protocol;

const uint32_t     TIMER_LINK_CHECK  = 1000 * 1;
const uint32_t     TIMER_PING   = 1000 * 2;

FPayClientCore::FPayClientCore():
	timer_link_check(this),
	timer_ping(this)
{

}

FPayClientCore::~FPayClientCore(){

}

//链接错误
void FPayFPayClientCoreCore::onError(int ev, const char *msg, IConn *conn)
{
	log( Info, "FPayClientCore::onError, conn:%d to node error:%s", 
				conn->getConnId(), msg );
	MultiConnManagerImp::onError(ev,msg,conn);
}

/*void FPayFPayClientCoreCore::onConnected(core::IConn* c)
  {
  log( Info, "FPayFPayClientCoreCore::onConnected, conn success cid:%d", c->getConnId() );
  }*/

//链接断开
void FPayFPayClientCoreCore::eraseConnect(IConn *conn)
{
	map<uint32_t,PROXY_INFO_T>::iterator it;
	it = cid2proxy.find(conn->getConnId());
	assert(it != cid2proxy.end());
	PROXY_INFO_T proxy = it->second;
	proxy2cid.erase(proxy);

	vector<uint32_t/*connid*/>::iterator acit;
	for( acit = all_cid.begin(); acit != all_cid.end(); ++acit )
	{
		if( *acit == conn->getConnId() )
		{
			all_cid.erase(acit);
			break;
		}
	}   

	IP_ADDR_T ip = get_ip_from_proxy_info(proxy);
	PORT_T port = get_port_from_proxy_info(proxy);
	unconnect_proxys.push_back(make_pair(ip,port));

	cid2proxy.erase(conn->getConnId());
	conn_timestamp_map.erase(conn->getConnId());

	vector<std::pair<IP_ADDR_T,PORT_T> >::iterator c_it;
	for( c_it = connected_proxys.begin(); c_it != connected_proxys.end(); )
	{
		if( c_it->first == ip && c_it->second == port )
		{
			c_it = connected_proxys.erase(c_it);
		}
		else
		{
			c_it++;
		}
	}

	log( Warn, "FPayFPayClientCoreCore::eraseConnect, delete conn(%d) to proxy(%s)",
				conn->getConnId(), proxy.c_str());
	MultiConnManagerImp::eraseConnect(conn);
}

void  FPayFPayClientCoreCore::connectAllProxy()
{
	log( Info, "FPayFPayClientCoreCore::connectAllProxy,unconnect_proxys size:%Zu", unconnect_proxys.size());
	IConn *conn = NULL;
	vector<std::pair<IP_ADDR_T,PORT_T> >::iterator it;
	for(it = unconnect_proxys.begin(); it != unconnect_proxys.end();)
	{
		conn = connectProxy(it->first,it->second);
		if( conn )
		{
			proxy2cid[format_proxy_info(it->first,it->second)] = conn->getConnId();
			cid2proxy[conn->getConnId()] = format_proxy_info(it->first,it->second);
			all_cid.push_back(conn->getConnId());
			connected_proxys.push_back(make_pair(it->first,it->second));

			conn_timestamp_map[conn->getConnId()] = time(NULL);

			if(init) registerOnConn( format_proxy_info(it->first,it->second),conn->getConnId() );

			log( Info, "FPayFPayClientCoreCore::connectAllProxy,connect to proxy(%s,%d) success connid:%d",
						it->first.c_str(), it->second, conn->getConnId() );
			it = unconnect_proxys.erase(it);
		}
		else
		{
			log(Error, "FPayFPayClientCoreCore::connectAllProxy,connect to proxy(%s,%d) error", it->first.c_str(), it->second);
			it++;
		}
	}
}

void FPayFPayClientCoreCore::startSV( const vector<pair<string,/*ip*/int32_t/*port*/> >& p )
{
	log(Info, "FPayFPayClientCoreCore::startSV,unconnect_proxys size:%Zu", p.size());
	unconnect_proxys = p;
	connectAllProxy();
	init = true;
	timer_link_check.start(TIMER_LINK_CHECK);
	timer_dispatch.start(TIMER_DISPATCH);
	timer_link_stat.start(TIMER_LINK_STAT);
	timer_ping.start(TIMER_PING);
	//timer_check_ping.start(TIMER_CHECK_PING);
}

void FPayFPayClientCoreCore::dispatchByProxy(const PROXY_INFO_T& proxy, core::Sender& s)
{
	s.endPack();
	lock.Lock();
	sender_queue.push_back(make_pair(proxy,s));
	lock.Unlock();
}

IConn *FPayFPayClientCoreCore::connectProxy( const IP_ADDR_T& ip, PORT_T port)
{
	log(Info, "FPayFPayClientCoreCore::connectProxy,host:[%s,%u]", ip.c_str(), port);
	IConn *conn = NULL;
	conn = createFPayFPayClientCoreCoreConn(ip.data(), port, handler, this);
	return conn;
}

std::string FPayFPayClientCoreCore::getServiceFlag()
{
	return name;
}

void FPayFPayClientCoreCore::setServiceFlag(const std::string& n)
{
	name = n;
}

bool FPayFPayClientCoreCore::linkCheck()
{
	log( Info, "FPayFPayClientCoreCore::linkCheck, check link wheath alive" );

	connectAllProxy();
	return true;
}

bool FPayFPayClientCoreCore::linkStat()
{
	log( Info,"FPayFPayClientCoreCore::linkStat,this:%p,conn_timestamp_map.size:%d,cid2proxy.size:%d,proxy2cid:%d,unconnect proxy size:%d, connect size:%d",
				this,conn_timestamp_map.size(),cid2proxy.size(), proxy2cid.size(),unconnect_proxys.size(), connected_proxys.size() );
	map<PROXY_INFO_T,uint32_t>::iterator it;
	for( it = proxy2cid.begin(); it != proxy2cid.end(); ++it )
	{
		log( Info, "FPayFPayClientCoreCore::linkStat,proxy:%s, connid:%d ", it->first.c_str(), it->second );
	}
	return true;
}

bool FPayFPayClientCoreCore::ping()
{
	log( Info, "FPayFPayClientCoreCore::ping,proxy2cid size:%Zu", proxy2cid.size());
	map<PROXY_INFO_T,uint32_t>::iterator it;
	for( it = proxy2cid.begin(); it != proxy2cid.end(); ++it)
	{
		PPingReq ping_req;
		ping_req.service_flag = getServiceFlag();

		Sender send;
			send.marshall(PPingReq::uri,ping_req);
			send.endPack();
			if( dispatchById( it->second, send ) == true )
			{
				//log( Info, "FPayFPayClientCoreCore::ping,connid:%d send ping to proxy:%s success",
				//        it->second, it->first.c_str() );
			}
			else
			{
				log( Warn, "FPayFPayClientCoreCore::ping,connid:%d send ping to proxy:%s failed",
						it->second, it->first.c_str() );
			}
		}
		return true;
	}

	/*bool FPayFPayClientCoreCore::checkPingTimeout()
	  {
	  log( Info, "FPayFPayClientCoreCore::checkPingTimeout" );
	  time_t now = time(NULL);
	  set<uint32_t> bad_conns;
	  map<uint32_t,uint32_t>::iterator it;
	  for( it = conn_timestamp_map.begin(); it != conn_timestamp_map.end(); ++it )
	  {
	//log( Info, "FPayFPayClientCoreCore::checkPingTimeout,conn:%d, timestamp:%d,now:%d",it->first,it->second, now );
	if( now - it->second  >=  PING_RESPONSE_TIMEOUT )
	{
	log( Info,"FPayFPayClientCoreCore::checkPingTimeout, conn:%d ping timeout ", it->first);
	bad_conns.insert(it->first);
	}
	}
	set<uint32_t>::iterator bad_it;
	for( bad_it = bad_conns.begin(); bad_it != bad_conns.end(); ++bad_it )
	{
	eraseConnectById( *bad_it );
	}
	return true;
	}*/


	void FPayFPayClientCoreCore::sendByProxy( const PROXY_INFO_T& proxy, core::Sender& send)
	{
		map<PROXY_INFO_T,uint32_t>::iterator it = proxy2cid.find(proxy);
		if(it != proxy2cid.end())
		{
			if(!dispatchById(it->second, send))
			{
				log( Error, "FPayFPayClientCoreCore::sendByProxy,send to proxy(%s) error,conn err", proxy.c_str());
			}
		}
		else
		{
			log( Error, "FPayFPayClientCoreCore::sendByProxy,send to proxy(%s) error,conn not found", proxy.c_str());
		}
	}

	/*void FPayFPayClientCoreCore::sendByRandom( core::Sender& send )
	  {
	  map<PROXY_INFO_T,uint32_t>::iterator it;
	  for( it = proxy2cid.begin(); it != proxy2cid.end(); ++it )
	  {
	  if(dispatchById(it->second, send))
	  {
	  return;
	  }
	  }
	  log( Error, "FPayFPayClientCoreCore::sendByRandom,send error,conn err");
	  }*/ //modidy 2014-3-19

	void FPayFPayClientCoreCore::sendByRandom( core::Sender& send )
	{
		poll++;
		if( !all_cid.empty() )
		{
			uint32_t idx = poll % all_cid.size();
			if(dispatchById(all_cid[idx], send))
			{
				return;
			}
		}

		log( Error, "FPayFPayClientCoreCore::sendByRandom,send error,conn err");
	}

	void FPayFPayClientCoreCore::sendByChannelRoll(uint32_t channel_id, core::Sender& send)
	{
		if( !all_cid.empty() )
		{
			uint32_t idx = channel_id % all_cid.size();
			if(dispatchById(all_cid[idx],send))
			{
				return;
			}
		}
		log( Error, "FPayFPayClientCoreCore::sendByChannelRoll,send error,conn err");
	}


	bool FPayFPayClientCoreCore::dispatch()
	{
		if( lock.TryLock() )
		{
			while( !sender_queue.empty() )
			{
				std::pair<PROXY_INFO_T,Sender> s = sender_queue.front();
				sender_queue.pop_front();

				if( s.first == RANDOM_DISPATCH_PROXY )
				{
					sendByRandom(s.second);
				}
				else
				{
					sendByProxy(s.first,s.second);
				}
			}
			lock.Unlock();
		}
		return true;
	}


	void FPayFPayClientCoreCore::response(const std::string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj)
	{
		PResponseRouter route;
		route.channel_id = channel_id;
		route.uid = uid;
		route.suid = suid;
		route.platform = platform;
		route.from = getServiceFlag();
		route.to = proxy;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PResponseRouter::uri, route);
		//debug
		if( proxy == "" )
		{
			log( Warn, "FPayFPayClientCoreCore::response, channel:%u,uid:%u,uri:%u", channel_id, uid, uri );
		}
		dispatchByProxy(proxy,s);
	}

	void FPayFPayClientCoreCore::multicast(uint32_t from_uid,  uint32_t channel_id, const set<uint32_t>& uids, uint32_t uri, sox::Marshallable& obj)
	{
		PMulticastRouter route;
		route.channel_id = channel_id;
		route.from_uid = from_uid;
		route.uids = uids;
		route.from = getServiceFlag();
		route.to = RANDOM_DISPATCH_PROXY;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PMulticastRouter::uri, route);
		dispatchByProxy(RANDOM_DISPATCH_PROXY,s);

	}

	void FPayFPayClientCoreCore::broadcastByChannel(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj)
	{
		PBroadcastByChannelRouter route;
		route.channel_id = channel_id;
		route.from = getServiceFlag();
		route.to = RANDOM_DISPATCH_PROXY;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PBroadcastByChannelRouter::uri, route);
		dispatchByProxy(RANDOM_DISPATCH_PROXY,s);
	}

	void FPayFPayClientCoreCore::broadcastBySubChannel( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj)
	{
		PBroadcastBySubChannelRouter route;
		route.channel_id = channel_id;
		route.subchannel_id = subchannel_id;
		route.from = getServiceFlag();
		route.to = RANDOM_DISPATCH_PROXY;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PBroadcastBySubChannelRouter::uri, route);
		dispatchByProxy(RANDOM_DISPATCH_PROXY,s);
	}


	void FPayFPayClientCoreCore::response_nolock(const std::string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj)
	{
		PResponseRouter route;
		route.channel_id = channel_id;
		route.uid = uid;
		route.suid = suid;
		route.platform = platform;
		route.from = getServiceFlag();
		route.to = proxy;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PResponseRouter::uri, route); 
		sendByProxy(proxy,s);
	}

	void FPayFPayClientCoreCore::broadcastByChannel_nolock(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj)
	{
		PBroadcastByChannelRouter route;
		route.channel_id = channel_id;
		route.from = getServiceFlag();
		route.to = RANDOM_DISPATCH_PROXY;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PBroadcastByChannelRouter::uri, route); 
		//sendByRandom(s);
		sendByChannelRoll(channel_id,s);
	}

	void FPayFPayClientCoreCore::broadcastBySubChannel_nolock( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj)
	{
		PBroadcastBySubChannelRouter route;
		route.channel_id = channel_id;
		route.subchannel_id = subchannel_id;
		route.from = getServiceFlag();
		route.to = RANDOM_DISPATCH_PROXY;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PBroadcastBySubChannelRouter::uri, route);
		//sendByRandom(s);
		sendByChannelRoll(channel_id,s); 
	}

	void FPayFPayClientCoreCore::ext_request(const std::string& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid,uint32_t uri,uint32_t msg_seq, uint8_t need_return, sox::Marshallable& obj,const std::string& dispatch_router_name)
	{
		PExtRequestRouter route;
		route.channel_id = channel_id;
		route.subchannel_id = subchannel_id;
		route.uid = uid;
		route.msg_seq = msg_seq;
		route.need_return = need_return;
		route.dispatch_router_name = dispatch_router_name;
		route.from = getServiceFlag();
		route.to = proxy;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PExtRequestRouter::uri, route);
		//debug
		if( proxy == "" )
		{
			log( Warn, "FPayFPayClientCoreCore::ext_request, channel:%u,uri:%u", channel_id,uri );
		}
		if( proxy == RANDOM_DISPATCH_PROXY ) {
			sendByRandom(s);
		} else {	
			sendByProxy(proxy,s);
		}
	}

	//query
	void FPayFPayClientCoreCore::query_by_channel_resp(const std::string& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj)
	{
		PQueryByChannelResp route;
		route.outside_connid = outside_connid;
		route.channel_id = channel_id;
		route.subchannel_id = subchannel_id;
		route.uid = uid;
		route.from = getServiceFlag();
		route.to = proxy;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PQueryByChannelResp::uri, route);
		//debug
		if( proxy == "" )
		{
			log( Warn, "FPayFPayClientCoreCore::query_by_channel_resp, channel:%u,uri:%u", channel_id,uri );
		}
		dispatchByProxy(proxy,s);

	}

	void FPayFPayClientCoreCore::query_by_uid_resp(const std::string& proxy, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj)
	{
		PQueryByUidResp route;
		route.outside_connid = outside_connid; 
		route.uid = uid;
		route.from = getServiceFlag();
		route.to = proxy;
		route.ruri = uri;
		route.packLoad(obj);
		Sender s(PQueryByUidResp::uri, route);
		//debug
		if( proxy == "" )
		{
			log( Warn, "FPayFPayClientCoreCore::query_by_uid_resp, uid:%u,uri:%u", uid,uri );
		}
		dispatchByProxy(proxy,s);
	}

	void FPayFPayClientCoreCore::report_user_info( uint8_t op, const user_info_t& user_info)
	{
		log( Info, "FPayFPayClientCoreCore::report_user_info: op:%u,uid:%u,channel:%u",op,user_info.uid,user_info.channel_id);
		PUserInfoReportRouter route;
		route.op = op;
		route.user_info.uid = user_info.uid;
		route.user_info.channel_id = user_info.channel_id;
		route.user_info.subchannel_id = user_info.subchannel_id;   
		route.user_info.json = user_info.json;

		Sender s(PUserInfoReportRouter::uri, route);
		std::map<PROXY_INFO_T,uint32_t>::iterator pit;
		for( pit = proxy2cid.begin(); pit != proxy2cid.end(); ++pit )
		{
			log( Info, "FPayFPayClientCoreCore::report_user_info: proxy:%s,op:%u,uid:%u,channel:%u",pit->first.c_str(),op,user_info.uid,user_info.channel_id);
			dispatchByProxy(pit->first,s);
		}
	}


