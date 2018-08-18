#ifndef __REDIS_CLIENT_H__
#define __REDIS_CLIENT_H__

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include "hiredis/hiredis.h"

using namespace std;

namespace redis {

typedef enum {
    kStatusOK          = 0,
    kStatusConnErr     = 1,
    kStatusProtoErr    = 2,
    kStatusKeyExistErr = 3,
    kStatusUndefineErr = 4
} RedisStatus;

typedef struct {
    std::string _host;
    uint16_t _port;
    std::string _passwd;
    struct timeval _timeout;
} RedisConf;

class RedisClient
{
    public:
        RedisClient()
            : _context(NULL)
        {
        }
        ~RedisClient()
        {
            disconnect();
        }
        bool init(const RedisConf& conf);
        bool isConnected()
        {
            return (_context != NULL);
        }
        bool ping();
        RedisStatus connect();
        RedisStatus get(const string& key, string& value);
        RedisStatus set(const string& key, const string& value, uint32_t duration, uint32_t x = 0);
        RedisStatus set(const char* key, uint32_t keySize,
					    const char* value, uint32_t valueSize, 
					    uint32_t duration = (uint32_t)-1, uint32_t x = 0);
        RedisStatus remove(const std::string& key);
        RedisStatus incrBy(const string& key, const int64_t increment, int64_t& rvalue);
        RedisStatus lpop(const string& key, string& value);
        RedisStatus rpush(const string& key, const vector<string>& values);
        RedisStatus lrange(const string& key, const int32_t start, const int32_t stop, vector<string>& members);
        RedisStatus sadd(const string& key, const string& value);
        RedisStatus smembers(const string& key, vector<string>& members);
        RedisStatus exec(const string& cmd, string& msg);
    
    private:
        void disconnect();
    
        RedisConf _conf;
        redisContext* _context;
};

}

#endif
