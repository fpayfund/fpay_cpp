#ifndef REDISPROTOCOL_H__
#define REDISPROTOCOL_H__
#include <string>
#include <vector>

namespace core {

class RedisProtocol
{
public:
    static void ConstructSubscribe(const std::string& key, std::string& packet);
    static int ParseSubscribeResponse(const char* data, size_t sz);
    static int ParseSubscribeMessage( const char* data, size_t sz, std::string& key, std::string& value );

    static void ConstructPublish(const std::string& key, const std::string& value, std::string& packet );
    static void ConstructHset(const std::string& key, const std::string& field, const std::string& value, std::string& packet );
    static void ConstructHmset(const std::string& key, const std::vector<std::pair<std::string,std::string> >& f2vs, std::string& packet );
    static void ConstructHdel(const std::string& key, const std::string& field, std::string& packet );
    static void ConstructHincrby(const std::string& key, const std::string& field, const std::string& value, std::string& packet );
    static void ConstructSet(const std::string& key,const std::string& value, std::string& packet );
    static void ConstructSadd(const std::string& key,const std::string& value, std::string& packet );

    static int ParseIntResponse(const char* data, size_t sz, int& _ret );
    static int ParseOkResponse(const char* data, size_t sz,std::string& out);
    static int ParseFailResponse(const char* data, size_t sz,std::string& err);

    static char ReturnType(const char* data);    
    static int ParseBlocksHead(const char* data, size_t sz, int& _block_count );
    static int ParseBlock(const char* data, size_t sz, std::string& _block);
protected:
    static int findCRLF(const char* data, size_t sz); 
    static int parseInt(const char* data, size_t sz, int& _ret);
    static int parseString(const char* data, size_t sz, std::string& _out);


    static char checkReplyType(const char* data);
    static int parseBlockLen(const char* data, size_t sz, int& _len);
    static int parseBlockData(const char* data, size_t sz, std::string& _data);
};
}

#endif

