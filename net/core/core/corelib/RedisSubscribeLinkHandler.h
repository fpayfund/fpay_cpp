#ifndef REDISSUBSCRIBELINKHANDLER_H__
#define REDISSUBSCRIBELINKHANDLER_H__
#include "common/core/ilink.h"
#include "RedisCallbackIf.h"
#include <vector>
#include <string>

namespace core {


class RedisSubscribeLinkHandler : public ILinkHandler
{
public:
    RedisSubscribeLinkHandler(IRedisSubscribeCallback* cb):
       status(kCmdSentOver),   
       subscribe_cb(cb)
    {
    }
    enum
    {
        kCmdSentOver           = 1,
        kCmdSentSuccessResp    = 2,
        kReceivedHead          = 3,
        kReceivedOver          = 4
    };
public:
    int onData(const char*, size_t, IConn *conn, int type = 0);
    void setPackLimit(uint32_t sz);
    uint32_t getPackLimit();
    void setStatus(uint32_t sts)
    {
        status = sts;
    }
protected: 
    uint32_t status;  
private:
    uint32_t packet_limit;
    IRedisSubscribeCallback* subscribe_cb;
};

}

#endif

