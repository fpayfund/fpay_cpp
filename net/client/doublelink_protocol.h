#ifndef DOUBLELINK_PROTOCOL_H_
#define DOUBLELINK_PROTOCOL_H_
#include "common/core/base_svid.h"
#include "common/packet.h"
#include "common/protocol/const.h"
#include <vector>

namespace doublelink {

	enum
	{
		DOUBLELINK_SID = 199
	};

	enum DoubleLinkProtoType
	{
		RESPONSE_ROUTE_PROTO                   = 1,
		MULTICAST_ROUTE_PROTO                 = 2,
		BROADCASTBYCHANNEL_ROUTE_PROTO        = 3,
		BROADCASTBYSUBCHANNEL_ROUTE_PROTO     = 4,
		REGISTER_PROTO                        = 5,
		UNREGISTER_PROTO                      = 6,
		REQUEST_ROUTE_PROTO                   = 7,
		//ping
		PING_REQUEST_PROTO                    = 8,
		PING_RESPONSE_PROTO                   = 9,

		//extend interface
		EXTEND_REQUEST_ROUTE_PROTO           = 10,
		EXTEND_RESPONSE_ROUTE_PROTO          = 11,
		//
		QUERY_BY_CHANNEL_REQ_PROTO             = 12,
		QUERY_BY_CHANNEL_RES_PROTO             = 13,
		QUERY_BY_UID_REQ_PROTO                 = 14, 
		QUERY_BY_UID_RES_PROTO                 = 15,
		QUERY_CONNID_REQ                       = 16,
		QUERY_CONNID_RESP                      = 17,

		// wb to proxy 
		USERINFO_REPORT_ROUTE_PROTO            = 18,
		GET_USERINFO_REQ_PROTO                 = 19,
		GET_USERINFO_RES_PROTO                 = 20,

		GET_SESSIONINFO_REQ_PROTO                 = 21,
		GET_SESSIONINFO_RES_PROTO                 = 22,

		END_SESSION_REQ_PROTO                     = 23,
		END_SESSION_RESP_PROTO                    = 24,

		//Thrift notice broadcast
		THRIFT_NOTICE_ROUTE_PROTO                 = 25,
		THRIFT_NOTICE_ROUTE_RESP_PROTO            = 26,
		//end session force
		END_SESSION_FORCE_REQ_PROTO               = 27,
		END_SESSION_FORCE_RESP_PROTO              = 28
	};

	//user info
	typedef struct _user_info {
		uint32_t uid;
		uint32_t channel_id;
		uint32_t subchannel_id;
		std::string json;
		time_t last_heartbeat;
	}user_info_t;

	typedef struct _session_info {
		std::string session;
		uint32_t channel_id;
		uint32_t subchannel_id;
		std::set<uint32_t> uids;
		time_t last_heartbeat;
	}session_info_t;

	typedef struct _channel_info {
		uint32_t channel_id;
		std::set<uint32_t> uids;
		time_t last_heartbeat;
	}channel_info_t;

	struct ChannelInterval : public sox::Marshallable
	{
		uint32_t begin;   //channel begin
		uint32_t end;     //channel end
		ChannelInterval():
			begin(0),
			end(0)
		{
		}
		ChannelInterval& operator=(const ChannelInterval& rhs)
		{
			this->begin = rhs.begin;
			this->end = rhs.end;
			return *this;
		}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << begin << end;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> begin >> end;
		}
	};

	inline bool operator < ( const struct ChannelInterval& l , const struct ChannelInterval& r )
	{
		if( l.end < r.begin )
		{
			return true;
		}
		return false;
	}

	inline bool operator == ( const struct ChannelInterval& l, const struct ChannelInterval& r )
	{
		if( l.begin <= r.begin && l.end >= r.end )
		{
			return true;
		}
		return false;
	}

	struct ChannelsSubscribeRegister: public sox::Marshallable
	{
		enum {uri = REGISTER_PROTO << 8 | DOUBLELINK_SID };

		std::string service_flag;
		std::string to;
		std::vector<ChannelInterval> channel_set;
		std::vector<ChannelInterval> backup_channel_set;
		virtual void marshal(sox::Pack &pk) const
		{
			pk << service_flag << to;
			marshal_container(pk, channel_set);
			marshal_container(pk, backup_channel_set);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> service_flag >> to;
			unmarshal_container(up, std::back_inserter(channel_set));
			if(up.size() > 0)
			{
				unmarshal_container(up,std::back_inserter(backup_channel_set));
			}
		}
	};

	struct ChannelsUnSubscribeRegister: public sox::Marshallable
	{
		enum {uri = UNREGISTER_PROTO << 8 | DOUBLELINK_SID };
		std::string service_flag;
		std::string to;
		std::vector<ChannelInterval> channel_set;
		virtual void marshal(sox::Pack &pk) const
		{
			pk << service_flag << to;
			marshal_container(pk, channel_set);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> service_flag >> to;
			unmarshal_container(up, std::back_inserter(channel_set));
		}
	};

	struct DoubleLinkRouterBase: public sox::Marshallable
	{
		std::string from;
		std::string to;
		uint32_t ruri;

		std::string load;
		DoubleLinkRouterBase():ruri(0){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << from << to << ruri;
			pk.push_varstr32(load.data(), load.size());
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> from >> to >> ruri;
			load = up.pop_varstr32();
		}
		void packLoad(const sox::Marshallable &obj)
		{
			sox::PackBuffer pb;
			sox::Pack pk(pb);
			pk << obj;
			load.assign(pk.data(), pk.size());
		}
	};

	struct PRequestRouter: DoubleLinkRouterBase
	{
		enum {uri = REQUEST_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;
		uint32_t uid;
		uint64_t suid;
		uint16_t platform;
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id << uid;
			pk << suid << platform;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id >> uid;
			if( !up.empty() )
			{
				up >> suid >> platform;
			}
		}
	};

	struct PResponseRouter: DoubleLinkRouterBase
	{
		enum {uri = RESPONSE_ROUTE_PROTO << 8 | DOUBLELINK_SID };

		uint32_t uid;
		uint32_t channel_id;
		uint64_t suid;
		uint16_t platform;
		PResponseRouter():uid(0),channel_id(0),suid(0),platform(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << uid << channel_id;
			pk << suid << platform;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> uid;
			if( up.size() > 0 )
			{
				up >> channel_id;
			}
			if( up.size() > 0)
			{
				up >> suid >> platform;
			}
		}
	};

	struct PMulticastRouter: DoubleLinkRouterBase
	{
		enum {uri = MULTICAST_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t from_uid;
		std::set<uint32_t> uids;
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << from_uid;
			marshal_container(pk, uids);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> from_uid;
			unmarshal_container(up, std::inserter(uids,uids.begin()));
		}
	};

	struct PBroadcastByChannelRouter: DoubleLinkRouterBase
	{
		enum {uri = BROADCASTBYCHANNEL_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id;
		}
	};

	struct PBroadcastBySubChannelRouter: DoubleLinkRouterBase
	{
		enum {uri = BROADCASTBYSUBCHANNEL_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id;
		}
	};

	struct PPingReq: public sox::Marshallable
	{
		enum {uri = PING_REQUEST_PROTO << 8 | DOUBLELINK_SID };
		std::string service_flag;
		virtual void marshal(sox::Pack &pk) const
		{
			pk << service_flag;
		}

		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> service_flag;
		}
	};

	struct PPingResp: public sox::Marshallable
	{
		enum {uri = PING_RESPONSE_PROTO << 8 | DOUBLELINK_SID };
		std::string service_flag;
		virtual void marshal(sox::Pack &pk) const
		{
			pk << service_flag;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> service_flag;
		}
	};

	struct PULServiceMsgHash10 : public sox::Marshallable  
	{
		enum { uri = 312 << 8 |88 };
		uint16_t serviceType;
		uint32_t sid;
		uint32_t uid;
		std::string msg;
		uint32_t uip;
		uint8_t	 terminalType;
		uint8_t  statType;

		PULServiceMsgHash10() : uip(0), terminalType(1) , statType(1){}

		virtual void marshal(sox::Pack &pak) const 
		{
			pak << serviceType << sid << uid;
			pak.push_varstr32(msg.data(), msg.size());
			pak << uip << terminalType << statType;
		}
		virtual void unmarshal(const sox::Unpack &pak) 
		{
			pak >> serviceType >> sid >> uid;
			msg = pak.pop_varstr32();
			if( !pak.empty() )
			{
				pak >> uip;
			}
			if (!pak.empty())
			{
				pak >> terminalType;
			}
			if(!pak.empty())
			{
				pak >> statType;
			}
		}
	};

	struct PExtRequestRouter: public DoubleLinkRouterBase
	{
		enum {uri = EXTEND_REQUEST_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;
		uint32_t uid;
		uint32_t msg_seq;
		uint8_t need_return;
		std::string dispatch_router_name;
		PExtRequestRouter():channel_id(0), subchannel_id(0),uid(0),msg_seq(0),need_return(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id << uid;
			pk << msg_seq;
			pk << need_return;
			pk << dispatch_router_name;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);    
			up >> channel_id >> subchannel_id >> uid;
			up >> msg_seq;
			up >> need_return;
			if( up.size() > 0 )
			{
				up >> dispatch_router_name;
			}
		}
	};

	struct PExtResponseRouter: DoubleLinkRouterBase
	{
		enum {uri = EXTEND_RESPONSE_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;
		uint32_t uid;
		uint32_t msg_seq;

		PExtResponseRouter():channel_id(0),subchannel_id(0),uid(0),msg_seq(0)
		{} 
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id << uid;
			pk << msg_seq;

		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id >> uid;
			up >> msg_seq;

		}
	};


	struct PQueryByChannelBase: public DoubleLinkRouterBase
	{
		uint32_t channel_id;
		uint32_t subchannel_id;
		uint32_t uid;
		uint32_t outside_connid;
		PQueryByChannelBase():channel_id(0), subchannel_id(0),uid(0),outside_connid(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id << uid << outside_connid;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id >> uid >> outside_connid;
		}
	};


	struct PQueryByChannelReq: public PQueryByChannelBase
	{
		enum {uri = QUERY_BY_CHANNEL_REQ_PROTO << 8 | DOUBLELINK_SID };    
	};

	struct PQueryByChannelResp: public PQueryByChannelBase
	{
		enum {uri = QUERY_BY_CHANNEL_RES_PROTO << 8 | DOUBLELINK_SID };  
	};

	struct PQueryByUidBase: public DoubleLinkRouterBase
	{ 
		uint32_t uid;
		uint32_t outside_connid;
		PQueryByUidBase():uid(0),outside_connid(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << uid << outside_connid;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> uid >> outside_connid;
		}
	};

	struct PQueryByUidReq: public PQueryByUidBase
	{
		enum {uri = QUERY_BY_UID_REQ_PROTO << 8 | DOUBLELINK_SID };
		PQueryByUidReq():req_connid(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			PQueryByUidBase::marshal(pk);
			pk << req_connid;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			PQueryByUidBase::unmarshal(up);
			up >> req_connid;
		}
		public:
		uint32_t req_connid;

	};

	struct PQueryByUidResp: public PQueryByUidBase
	{
		enum {uri = QUERY_BY_UID_RES_PROTO << 8 | DOUBLELINK_SID };
	};

	struct PQueryConnIdReq: public sox::Marshallable
	{
		enum {uri = QUERY_CONNID_REQ << 8 | DOUBLELINK_SID }; 
		virtual void marshal(sox::Pack &pk) const
		{

		}
		virtual void unmarshal(const sox::Unpack &up)
		{

		}

	};

	struct PQueryConnIdResp: public sox::Marshallable
	{
		enum {uri = QUERY_CONNID_RESP << 8 | DOUBLELINK_SID };      
		virtual void marshal(sox::Pack &pk) const
		{
			marshal_container(pk, connids);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{ 
			unmarshal_container(up, std::back_inserter(connids));
		}
		public:
		std::vector<uint32_t> connids;
	};

	struct UserInfo :public sox::Marshallable
	{
		UserInfo():uid(0),channel_id(0),subchannel_id(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{   
			pk << uid << channel_id << subchannel_id << json;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{   
			up >> uid >> channel_id >> subchannel_id >> json;
		}
		public:
		uint32_t uid;
		uint32_t channel_id;
		uint32_t subchannel_id;
		std::string json;
	};

	struct PUserInfoReportRouter: public sox::Marshallable
	{
		enum {uri = USERINFO_REPORT_ROUTE_PROTO << 8 | DOUBLELINK_SID };

		PUserInfoReportRouter():op(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{  
			pk << op << user_info;  
		}
		virtual void unmarshal(const sox::Unpack &up)
		{  
			up >> op >> user_info;
		}
		public:
		uint8_t op; //0 add 1 for remove
		UserInfo user_info;
	};

	struct PGetUserInfoReq: public sox::Marshallable
	{
		enum {uri = GET_USERINFO_REQ_PROTO << 8 | DOUBLELINK_SID };
		PGetUserInfoReq():uid(0) {}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << uid; 
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> uid;
		}
		public:
		uint32_t uid;
	};

	struct PGetUserInfoResp: public sox::Marshallable
	{
		enum {uri = GET_USERINFO_RES_PROTO << 8 | DOUBLELINK_SID };
		PGetUserInfoResp():uid(0),channel_id(0),subchannel_id(0) {}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << uid << channel_id << subchannel_id;
			marshal_container(pk, uids);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> uid >> channel_id >> subchannel_id;
			unmarshal_container(up, std::back_inserter(uids));
		}
		public:
		uint32_t uid;
		uint32_t channel_id;
		uint32_t subchannel_id;
		std::vector<uint32_t> uids;
	};

	struct PGetSessionInfoReq: public sox::Marshallable
	{
		enum {uri = GET_SESSIONINFO_REQ_PROTO << 8 | DOUBLELINK_SID };
		PGetSessionInfoReq(){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << session;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session;
		}
		public:
		std::string session;
	};

	struct PGetSessionInfoResp: public sox::Marshallable
	{
		enum {uri = GET_SESSIONINFO_RES_PROTO << 8 | DOUBLELINK_SID };
		PGetSessionInfoResp():channel_id(0),subchannel_id(0) {}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << channel_id << subchannel_id;
			marshal_container(pk, uids);
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> channel_id >> subchannel_id;
			unmarshal_container(up, std::back_inserter(uids));
		}
		public:
		std::string session;
		uint32_t channel_id;
		uint32_t subchannel_id;
		std::vector<uint32_t> uids;
	};

	struct PEndSessionReq: public sox::Marshallable
	{
		enum {uri = END_SESSION_REQ_PROTO << 8 | DOUBLELINK_SID };
		PEndSessionReq(){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << session << lesson_id;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> session >> lesson_id;
		}
		public:
		std::string session;
		std::string lesson_id;
	};

	struct PEndSessionResp: public sox::Marshallable
	{
		enum {uri = END_SESSION_RESP_PROTO << 8 | DOUBLELINK_SID };
		PEndSessionResp(){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk <<  resp_code; 
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> resp_code; 
		}
		public:
		uint32_t resp_code;
	};

	struct PThriftNoticeRouter: public DoubleLinkRouterBase
	{
		enum {uri = THRIFT_NOTICE_ROUTE_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;

		PThriftNoticeRouter():channel_id(0), subchannel_id(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id;
		}
	};

	struct PThriftNoticeRouterResp: public sox::Marshallable
	{
		enum {uri = THRIFT_NOTICE_ROUTE_RESP_PROTO << 8 | DOUBLELINK_SID };
		PThriftNoticeRouterResp(){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << lesson_id <<  resp_code;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> lesson_id >> resp_code;
		}
		public:
		std::string lesson_id;
		uint32_t resp_code;
	};

	struct PEndSessionForceRouter: public DoubleLinkRouterBase
	{
		enum {uri = END_SESSION_FORCE_REQ_PROTO << 8 | DOUBLELINK_SID };
		uint32_t channel_id;
		uint32_t subchannel_id;

		PEndSessionForceRouter():channel_id(0), subchannel_id(0)
		{}
		virtual void marshal(sox::Pack &pk) const
		{
			DoubleLinkRouterBase::marshal(pk);
			pk << channel_id << subchannel_id;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			DoubleLinkRouterBase::unmarshal(up);
			up >> channel_id >> subchannel_id;
		}
	};

	struct PEndSessionForceRouterResp: public sox::Marshallable
	{
		enum {uri = END_SESSION_FORCE_RESP_PROTO << 8 | DOUBLELINK_SID };
		PEndSessionForceRouterResp(){}
		virtual void marshal(sox::Pack &pk) const
		{
			pk << lesson_id <<  resp_code;
		}
		virtual void unmarshal(const sox::Unpack &up)
		{
			up >> lesson_id >> resp_code;
		}
		public:
		std::string lesson_id;
		uint32_t resp_code;
	};

}
#endif /*DOUBLELINK_PROTOCOL_H_*/
