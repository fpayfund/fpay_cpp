#ifndef REDISCALLBACKIF_H__
#define REDISCALLBACKIF_H__
#include <string>

namespace core {

struct IRedisSubscribeCallback
{
public:
    virtual ~IRedisSubscribeCallback() 
    {}
    virtual void SubscribeCallback(const std::string& key, const std::string& value) = 0;
};

}
#endif

