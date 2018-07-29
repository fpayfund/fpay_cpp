#include <stdlib.h>
#include <sstream>
#include <memory.h>
#include <iostream>
#include <vector>

#include "RedisClient.h"

namespace redis {

static const string kTextOK  = "OK";

template <typename s, typename t>
std::string lexical_cast(t num)
{
    std::ostringstream os;
    os << num;
    return os.str();
}

bool RedisClient::init(const RedisConf& conf)
{
    _conf = conf;
    if (connect() != kStatusOK) {
        return false;
    }
    return true;
}

RedisStatus RedisClient::connect()
{
    if (_context) {
        disconnect();
    }

    _context = redisConnectWithTimeout(_conf._host.c_str(), _conf._port, _conf._timeout);
    if (_context && _context->err) {
        disconnect();
        return kStatusConnErr;
    }

    redisSetTimeout(_context, _conf._timeout);

    if (_conf._passwd.size() > 2) {
        string pwd = _conf._passwd;
        for(size_t i = 0; i < pwd.size() && i < 6; ++i) {
            pwd[i] = '*';    // cover some part of pwd
        }
        string cmd = "AUTH ";
        cmd += _conf._passwd;
        redisReply* reply = static_cast<redisReply*>( redisCommand( _context, cmd.c_str() ) );
        if ( !reply ) {
            disconnect();
            return kStatusConnErr;
        }
        string result;
        if ( reply->type == REDIS_REPLY_STATUS) {
            result = reply->str;
        } else if ( reply->type == REDIS_REPLY_ARRAY ) {
            // for ssdb compatibable
            redisReply* item = NULL;
            std::string val;
            for ( size_t i = 0; i < reply->elements; ++i ) {
                item = static_cast<redisReply*>( reply->element[i] );
                if ( item->type == REDIS_REPLY_STRING ) {
                    if ( std::string(item->str) == "ok" || std::string(item->str) == "OK" || std::string(item->str) == "1" ) {
                        result = "OK";
                        break;
                    }
                }
            }
        }

        freeReplyObject(reply);
        if (result != "OK") {
            string passwd = _conf._passwd;
            passwd[1] = '*';
            passwd[2] = '*';
            passwd[3] = '*';
            return kStatusProtoErr;
        }
    }

    return kStatusOK;
}

void RedisClient::disconnect()
{
    if (!_context) {
        return;
    }

    redisFree(_context);
    _context = NULL;
}

bool RedisClient::ping()
{
    if (!_context) {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context, "ping"));
    if (!reply) {
        disconnect();
        return false;
    }

    bool ret = false;
    if (reply->type == REDIS_REPLY_STATUS && reply->str == "PONG") {
        ret = true;
    }
  
    freeReplyObject( reply );
    return ret;
}

RedisStatus RedisClient::get(const string& key, string& value)
{
    if (!_context) {
        return kStatusConnErr;
    }

    if (key.empty()) {
        return kStatusProtoErr;
    }

    redisReply* reply = static_cast<redisReply*>( redisCommand( _context, "GET %b", key.data(), key.size()) );
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if ( reply->type == REDIS_REPLY_STRING ) {
        value.assign(reply->str, reply->len);
    }

    freeReplyObject( reply );
    return kStatusOK;
}

RedisStatus RedisClient::set(const string& key, const string& value, uint32_t duration, uint32_t x)
{
    if (!_context) {
        return kStatusConnErr;
    }

    if (x > 2 || key.empty()) {
        return kStatusProtoErr;
    }

    redisReply* reply = NULL;
    if (duration != 0) {
        string cmd = "SET %b %b EX %u";
        if (x != 0) {
            cmd += (x == 1 ? " XX" : " NX");
        }
        reply = static_cast<redisReply*>( redisCommand( _context, cmd.c_str(), key.data(), key.size(), value.data(), value.size(), duration ) );
    } else {
        string cmd = "SET %b %b";
        if (x != 0) {
            cmd += (x == 1 ? " XX" : " NX");
        }
        reply = static_cast<redisReply*>( redisCommand( _context, cmd.c_str(), key.data(), key.size(), value.data(), value.size()) );
    }

    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if (x == 2 && reply->type == REDIS_REPLY_NIL) {
        // when set option NX and record exists will return NULL
        return kStatusKeyExistErr;
    }

    RedisStatus ret = kStatusUndefineErr;
    //if (reply->type == REDIS_REPLY_STATUS && reply->str == "OK") { 
    if (reply->type == REDIS_REPLY_STATUS && reply->str == kTextOK) { 
        ret = kStatusOK;
    }

    freeReplyObject( reply );
    return ret;
}

RedisStatus RedisClient::remove(const std::string& key)
{
    if (!_context) {
        return kStatusConnErr;
    }

    if (key.empty()) {
        return kStatusProtoErr;
    }

    std::string cmd = "DEL";
    size_t argc = 2;
    const char* argv[2];
    size_t argvlen[2];

    argv[0] = cmd.data();
    argvlen[0] = cmd.length();
    argv[1] = key.data();
    argvlen[1] = key.length();

    redisReply* reply = static_cast<redisReply*>( redisCommandArgv( _context, argc, argv, argvlen ) );
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    freeReplyObject( reply );
    return kStatusOK;
}

RedisStatus RedisClient::incrBy(const string& key, const int64_t increment, int64_t& rvalue)
{
    if (!_context) {
        return kStatusConnErr;
    }

    if (key.empty()) {
        return kStatusProtoErr;
    }

    // INCRBY key increment
    std::string cmd = "INCRBY";
    size_t argc = 3;
    const char* argv[argc];
    size_t argvlen[argc];

    argv[0] = cmd.data();
    argvlen[0] = cmd.length();
    argv[1] = key.data();
    argvlen[1] = key.length();
    string incr_text = lexical_cast<std::string>(increment);
    argv[2] = incr_text.data();
    argvlen[2] = incr_text.length();

    redisReply* reply = static_cast<redisReply*>( redisCommandArgv( _context, argc, argv, argvlen ) );
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    RedisStatus ret = kStatusUndefineErr;
    if (reply->type == REDIS_REPLY_INTEGER) {
        rvalue = reply->integer;
        ret = kStatusOK;
    }

    freeReplyObject( reply );
    return ret;
}

RedisStatus RedisClient::lpop(const string& key, string& value)
{
    if (!_context) {
        return kStatusConnErr;
    }

    size_t argc = 2;
    const char* argv[argc];
    size_t argvlen[argc];
  
    std::string cmd = "LPOP";
    argv[0]    = cmd.data();
    argvlen[0] = cmd.length();
    argv[1]    = key.data();
    argvlen[1] = key.length();
  
    redisReply* reply = static_cast<redisReply*>( redisCommandArgv( _context, argc, argv, argvlen ) );
  
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if ( reply->type == REDIS_REPLY_STRING ) {
        value.assign(reply->str, reply->len);
    }
  
    freeReplyObject( reply );
    return kStatusOK;
}

RedisStatus RedisClient::rpush(const string& key, const vector<string>& vals)
{
    if (!_context) {
        return kStatusConnErr;
    }

    if (key.empty() || vals.empty()) {
        return kStatusProtoErr;
    }

    size_t argc = vals.size() + 2;
    const char* argv[argc];
    size_t argvlen[argc];

    std::string cmd = "RPUSH";
    argv[0] = cmd.data();
    argvlen[0] = cmd.length();
    argv[1] = key.data();
    argvlen[1] = key.length();

    size_t i = 2;
    for (vector< string >::const_iterator cit = vals.begin(); cit != vals.end(); ++cit) {
        argv[i] = cit->data();
        argvlen[i] = cit->length();
        ++i;
    }

    redisReply* reply = static_cast<redisReply*>( redisCommandArgv( _context, argc, argv, argvlen ) );

    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    RedisStatus ret = kStatusUndefineErr;
    if (reply->type == REDIS_REPLY_INTEGER) {
        ret = kStatusOK;
    }

    freeReplyObject(reply);
    return ret;
}

RedisStatus RedisClient::lrange(const string& key, const int32_t start, const int32_t stop, vector<string>& members)
{
    if (!_context) {
        return kStatusConnErr;
    }

    //  LRANGE key start stop
    std::string cmd = "LRANGE";

    int argc = 4;
    char* argv[ argc ];
    size_t argvlen[ argc ];

    argv[0] = const_cast<char*>(cmd.data());
    argvlen[0] = cmd.length();
    argv[1] = const_cast<char*>(key.data());
    argvlen[1] = key.length();
    string start_text = lexical_cast<string>(start);
    string stop_text  = lexical_cast<string>(stop);
    argv[2] = const_cast<char*>(start_text.data());
    argvlen[2] = start_text.length();
    argv[3] = const_cast<char*>(stop_text.data());
    argvlen[3] = stop_text.length();

    redisReply* reply = static_cast<redisReply*>(redisCommandArgv(_context, argc, (const char**)(&argv[0]), &argvlen[0]));
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if (reply->type != REDIS_REPLY_ARRAY) {
        freeReplyObject( reply );
        return kStatusUndefineErr;
    }

    redisReply* item = NULL;
    string val;
    for (size_t i = 0; i < reply->elements; ++i) {
        item = static_cast<redisReply*>( reply->element[i] );
        if (item->type == REDIS_REPLY_STRING) {
            val.assign(item->str, item->len);
            members.push_back(val);
        }
    }
    freeReplyObject(reply);

    return kStatusOK;
}

RedisStatus RedisClient::sadd(const string& key, const string& value)
{
    if (!_context) {
        return kStatusConnErr;
    }

    redisReply* reply = static_cast<redisReply*>( redisCommand( _context, "SADD %b %b", key.data(), key.size(), value.data(), value.size()) );
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if (reply->type != REDIS_REPLY_INTEGER) {
        freeReplyObject( reply );
        return kStatusUndefineErr;
    }
    freeReplyObject( reply );

    return kStatusOK;
}

RedisStatus RedisClient::smembers(const string& key, vector<string>& members)
{
    if (!_context) {
        return kStatusConnErr;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(_context, "SMEMBERS %b", key.data(), key.size()));
    if (!reply) {
        disconnect();
        return kStatusConnErr;
    }

    if (reply->type != REDIS_REPLY_ARRAY) {
        freeReplyObject( reply );
        return kStatusUndefineErr;
    }

    redisReply* item = NULL;
    string val;
    for (size_t i = 0; i < reply->elements; ++i) {
        item = static_cast<redisReply*>( reply->element[i] );
        if (item->type == REDIS_REPLY_STRING) {
            val.assign(item->str, item->len);
            members.push_back(val);
        }
    }
    freeReplyObject(reply);

    return kStatusOK;
}

RedisStatus RedisClient::exec(const string& cmd, string& msg)
{
    if (!_context) {
        return kStatusConnErr;
    }

    redisReply* reply = static_cast<redisReply*>( redisCommand( _context, cmd.c_str() ) );
    if ( !reply ) {
        disconnect();
        return kStatusConnErr;
    } else if ( reply->type == REDIS_REPLY_ERROR ) {
        msg = reply->str;
        freeReplyObject( reply );
        return kStatusUndefineErr;
    } else if (reply->type == REDIS_REPLY_STRING) {
        msg = reply->str;
        freeReplyObject(reply);
        return kStatusOK;
    }

    freeReplyObject(reply);
    msg = "OK";

    return kStatusOK;
}

}
