
#ifndef DOUBLELINK_INETSENDER_H_
#define DOUBLELINK_INETSENDER_H_
#include "common/core/request.h"
#include "doublelink_protocol.h"
#include "doublelinkdefine.h" 
#include <set>
namespace doublelink {

	struct IDispatchResponse
	{
		//response or broadcast interface
		virtual void response(const std::string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj) = 0;
		virtual void multicast(uint32_t from_uid, uint32_t channel_id, const std::set<uint32_t>& uids, uint32_t uri, sox::Marshallable& obj) = 0;
		virtual void broadcastByChannel(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj) = 0;
		virtual void broadcastBySubChannel( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj) = 0;


		virtual void response_nolock(const std::string& proxy, uint32_t channel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, sox::Marshallable& obj) = 0;  
		virtual void broadcastByChannel_nolock(uint32_t channel_id, uint32_t uri, sox::Marshallable& obj) = 0;
		virtual void broadcastBySubChannel_nolock( uint32_t channel_id, uint32_t subchannel_id,uint32_t uri, sox::Marshallable& obj) = 0;

		//extend interface
		virtual void ext_request(const std::string& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t msg_seq, uint8_t need_return, sox::Marshallable& obj,const std::string& dispatch_router_name) = 0;

		//query
		virtual void query_by_channel_resp(const std::string& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj) = 0;
		virtual void query_by_uid_resp(const std::string& proxy, uint32_t uid, uint32_t uri, uint32_t outside_connid,sox::Marshallable& obj) = 0;

		virtual void report_user_info(uint8_t op, const user_info_t& user_info) = 0;
	};

	struct IDispatchResponseAware
	{
		protected:
			IDispatchResponse* response;
		public:
			inline IDispatchResponse* getDispatchResponse() const
			{
				return response;
			}
			void setDispatchResponse(IDispatchResponse* r)
			{
				response = r;
			}
	};

}
#endif

