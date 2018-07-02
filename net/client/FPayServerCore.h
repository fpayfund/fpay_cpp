#ifndef DOUBLELINK_ROUTER_H_
#define DOUBLELINK_ROUTER_H_
#include "doublelink_protocol.h"
#include "common/core/ibase.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "common/core/ilink.h"
#include <boost/shared_ptr.hpp>
//service
#include "BroadcasterClient2.h"
#include "ServiceProxyLayer.h" 
#include "UnicastClient.h"
#include "ChannelRange.h"

namespace doublelink {

	struct IDispatchRequest
	{
		virtual void dispatchRequest(uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, const string& load) = 0;
	};

	struct IDispatchRequestAware
	{
		protected:
			IDispatchRequest* dispatcher;
		public:
			inline IDispatchRequest* getDispatchRequest() const
			{
				return dispatcher;
			}
			void setDispatchRequest(IDispatchRequest* r)
			{
				dispatcher = r;
			}
	};

	class DoubleLinkRouter:
		public core::PHClass, 
		public core::IFormTarget,
		public core::MultiConnManagerImp,
		public IDispatchRequest,
		public core::IServerIdDispatcherAware
	{
		typedef struct conn_info
		{
			uint32_t cid;
			string proxy;
			time_t last_ping_time;
			uint64_t req_count;
			conn_info()
			{
				cid = 0;
				last_ping_time = time(NULL);
				req_count = 0;
			}
		}conn_info_t;
		typedef struct service_info
		{
			string service_flag;
			uint64_t sid;
			vector<ChannelInterval> channel_set;
			vector<ChannelInterval> backup_channel_set;
			vector<conn_info_t> conns;
		}service_info_t;

		public:
		DECLARE_FORM_MAP

		private:
			void onRegister(ChannelsSubscribeRegister *reg, core::IConn* c);
			void onUnRegister(ChannelsUnSubscribeRegister *unreg, core::IConn* c);
			void onResponse(PResponseRouter* r, core::IConn* c);
			void onMulticast(PMulticastRouter* r,core::IConn* c);
			void onBroadcastByChannel(PBroadcastByChannelRouter* r,core::IConn* c);
			void onBroadcastBySubChannel(PBroadcastBySubChannelRouter* r,core::IConn* c);
			//extend if
			void onExtRequest(PExtRequestRouter* r, core::IConn* c);
			void onExtResponse(PExtResponseRouter* r, core::IConn* c);

			//query
			void onQueryByChannelRequest(PQueryByChannelReq* r, core::IConn* c);
			void onQueryByChannelResponse(PQueryByChannelResp* r, core::IConn* c);

			void onQueryByUidRequest(PQueryByUidReq* r, core::IConn* c);
			void onQueryByUidResponse(PQueryByUidResp* r, core::IConn* c);

			void onQueryConnidsRequest(PQueryConnIdReq* r, core::IConn* c);

			void onUserInfoReport(PUserInfoReportRouter* r,core::IConn* c);
			void onGetUserInfoReq(PGetUserInfoReq* r, core::IConn* c);


			void onGetSessionInfoReq(PGetSessionInfoReq* r, core::IConn* c);
			void onEndSessionReq(PEndSessionReq* r, core::IConn* c);

			void onThriftNotice(PThriftNoticeRouter* r, core::IConn* c);
			void onEndSessionForce(PEndSessionForceRouter* r, core::IConn* c);
		
			
			//ping 
			void onPing(PPingReq* p, core::IConn* c);
			void onPULServiceMsgHash2(PULServiceMsgHash10* r,core::IConn* c);
		public:
			DoubleLinkRouter(server::service::ServiceProxyLayer& spl,server::service::BroadcasterClient2& bc, 
					server::service::UnicastClient& uc );
			virtual ~DoubleLinkRouter();
			virtual void dispatchRequest(uint32_t channel_id, uint32_t uid,  uint64_t suid, uint16_t platform, uint32_t uri, const string& load);

		protected:


			virtual void eraseConnect(IConn *conn);
			bool findServiceByChannel( uint32_t channel_id, string& service_flag ); 
			//stat
			bool showStatInfo();
			// ping check
			bool checkPingTimeout();
			bool showRegister();    

			bool findMasterServiceByChannel( uint32_t channel_id, string& service_flag );
			bool findSlaveServiceByChannel( uint32_t channel_id, string& service_flag );

			uint64_t findMasterSidByChannel( uint32_t channel_id );
			uint64_t findSlaveSidByChannel( uint32_t channel_id );
			uint64_t findSidByConnId( uint32_t cid );
		private:
			void chanagePingTimestamp(uint32_t cid);
			void pingResponse( const string& service, uint32_t cid);

		protected:
			map<uint32_t,/*connid*/string/*service_flag*/> cid2service;
			map<uint32_t,/*connid*/uint64_t/*response count*/> cid2responses;

			ChannelRange channel2Sid;
			ChannelRange backupChannel2Sid;
			map<uint64_t,string/*service flag*/> sid2ServiceFlag;
			map<string/*service flag*/,service_info_t> service2conns; 
			uint64_t sid;
			//serivce sdk
			server::service::ServiceProxyLayer* proxy_layer_;
			server::service::BroadcasterClient2* broadcast_cli_;
			server::service::UnicastClient* unicast_cli_;

			//ext query conn
			std::set<uint32_t> pending_query_conns_;

			//user info
			std::map<uint32_t/*uid*/,user_info_t> uid2userinfo_; 
			std::map<uint32_t/*channel_id*/,channel_info_t> channel_2_uids_;
			std::map<string/*session*/,session_info_t> session2sessioninfo_;

			bool cleanExpireUidInfo(); 
			bool cleanExpireChannelInfo();
			bool cleanExpireSessionInfo();
		private:
			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::showStatInfo> timer_stat;
			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::checkPingTimeout> timer_check_ping;
			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::showRegister> timer_show_register;

			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::cleanExpireUidInfo> timer_clean_uidinfo;
			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::cleanExpireChannelInfo> timer_clean_channelinfo;
			TimerHandler<DoubleLinkRouter, &DoubleLinkRouter::cleanExpireSessionInfo> timer_clean_sessioninfo;    

			uint64_t response_count;
			uint64_t channel_broadcast_count;
			uint64_t subchannel_broadcast_count;
			uint64_t multicast_count;
			string dispatch_route_name_;
	};

}

#endif

