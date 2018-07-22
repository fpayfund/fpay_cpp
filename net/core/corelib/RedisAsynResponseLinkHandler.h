#ifndef REDISASYNRESPONSELINKHANDLER_H__
#define REDISASYNRESPONSELINKHANDLER_H__
#include "common/core/ilink.h"
#include "RedisCallbackIf.h"
#include <vector>
#include <string>

namespace core {


class RedisAsynResponseLinkHandler : public ILinkHandler
{
public:
    RedisAsynResponseLinkHandler()
    {
    }
public:
    int onData(const char*, size_t, IConn *conn, int type = 0);
    void setPackLimit(uint32_t sz);
    uint32_t getPackLimit();

private:
    uint32_t packet_limit;
};

}

#endif

