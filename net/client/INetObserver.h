#ifndef DOUBLELINK_INETOBSERVER_H_
#define DOUBLELINK_INETOBSERVER_H_
#include "common/core/request.h"
#include "doublelinkdefine.h" 
namespace doublelink {

	// observer net interface
	struct INetObserver
	{
		virtual void handleRequest( const PROXY_INFO_T& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint64_t suid, uint16_t platform, uint32_t uri, core::Request& req ) = 0;
		virtual void handleRequest( const PROXY_INFO_T& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, sox::Unpack& up ) = 0;

		virtual void handleQueryByChannelRequest(const PROXY_INFO_T& proxy, uint32_t channel_id, uint32_t subchannel_id, uint32_t uid, uint32_t uri, uint32_t outside_connid, sox::Unpack& up ) = 0;

		virtual void handleQueryByUidRequest(const PROXY_INFO_T& proxy,uint32_t uid, uint32_t uri, uint32_t outside_connid, sox::Unpack& up ) = 0;
		virtual void handleEndSessionRequest(const PROXY_INFO_T& proxy, const std::string& session, const std::string& lesson_id) = 0;
	};

}

#endif

