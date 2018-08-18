#ifndef __CACHE_H__
#define __CACHE_H__

#include "RedisClient.h"
#include "hiredis/hiredis.h"

using namespace std;
using namespace redis;

class Cache
{
    public:
        Cache() {}
        ~Cache();
    
        bool init(const string& confPath);
        void checkRedis();
        bool get(const string& key, string& value);
        bool set(const string& key, const string& value, uint32_t duration);
        bool set(const char* key, uint32_t keySize, const char* value,
				 uint32_t valueSize, uint32_t duration = (uint32_t)-1);
        bool setNX(const string& key, const string& value);
        bool incrBy(const string& key, const int64_t increment, int64_t& rvalue);
        bool remove(const string& key);
        bool enqueue(const string& topic, const string& value);
        bool enqueue(const string& topic, const std::vector<string>& values);
        bool dequeue(const string& topic, string& value);
        bool dequeueAll(const string& topic, std::vector<string>& members);
        bool addToSet(const string& key, const string& value);
        bool getMembers(const string& key, vector<string>& members);
    
        static Cache* create(const string& confPath)
        {
            Cache* cache = new Cache();
            if (!cache->init(confPath)) {
                delete cache;
                return NULL;
            }
            return cache;
        }
    
    private:
        bool loadConf(const string& confPath, std::vector<RedisConf> &confArray);
        RedisClient* dispatch(const string& key);
    
        std::vector<RedisClient*> _clientPool;
};

#endif
