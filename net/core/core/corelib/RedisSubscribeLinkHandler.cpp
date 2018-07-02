#include  "RedisSubscribeLinkHandler.h"
#include "RedisProtocol.h"

namespace core {

using namespace std;

static const uint32_t kSubscribeResponseBlockNum = 3;

int RedisSubscribeLinkHandler::onData(const char* data, size_t sz, IConn *conn, int type)
{
    int ret = 0;
    if( status == kCmdSentOver )
    {
        ret = RedisProtocol::ParseSubscribeResponse(data, sz);
        if( ret == -1 )
        {
            return -1;
        }
        else if( ret == 0 )
        {
            return 0;
        }
        status = kCmdSentSuccessResp;
        return ret; 
    }
    size_t real_ret = 0;
loop:
    if( status == kCmdSentSuccessResp )
    {
        string key,value;
        ret = RedisProtocol::ParseSubscribeMessage(data+real_ret,sz-real_ret,key,value );
        if( ret == 0 )
        {
            return real_ret;
        }
        else if( ret == -1 )
        {
            return -1;
        }

        status = kCmdSentSuccessResp;
        subscribe_cb->SubscribeCallback(key,value);
        real_ret += ret;
        if( real_ret < sz )
        {
           goto loop;
        }
        return real_ret;
    }
    return -1;
}

void RedisSubscribeLinkHandler::setPackLimit(uint32_t sz)
{
    packet_limit = sz;
}

uint32_t RedisSubscribeLinkHandler::getPackLimit()
{
    return packet_limit;
}

}
