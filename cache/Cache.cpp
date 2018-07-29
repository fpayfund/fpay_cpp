#include <iostream>
#include <string>
#include "tinyxml/tinyxml.h"
#include "Cache.h"

using namespace std;
using namespace imtixml;

Cache::~Cache()
{
    for (uint32_t i = 0; i < _clientPool.size(); i++) {
		RedisClient* client = _clientPool[i];
        delete client;
    }

	_clientPool.clear();
}

bool Cache::init(const string& confPath)
{
    vector<RedisConf> confArray;
    if (!loadConf(confPath, confArray)) {
        return false;
    }

    for (uint32_t i = 0; i < confArray.size(); i++) {
		RedisConf &conf = confArray[i];

        RedisClient* client = new RedisClient();
        if (!client->init(conf)) {
            delete client;
        }

        _clientPool.push_back(client);
    }

    return !_clientPool.empty();
}

RedisClient* Cache::dispatch(const string& key)
{
	uint32_t hash = 1315423911;
    if (!key.empty()) {
        const char* start = key.c_str();
        const char* end = start + key.size();
        for(const char* key = start + 7; key < end; key++) {
	        hash ^= ((hash << 5) + (*key) + (hash >> 2));
	    }
    }
    hash = (hash & 0x7FFFFFFF) % _clientPool.size();

    RedisClient* client = _clientPool[hash];

    if (!client->isConnected()) {
        RedisStatus status = client->connect();
        if (status != kStatusOK) {
            return NULL;
        }
    }

    return client;
}

bool Cache::loadConf(const string& confPath, std::vector<RedisConf> &confArray)
{
    if (confPath.empty()) {
        return false;
    }

    FILE* fp = fopen(confPath.c_str(), "r");
    if (!fp) {
        return false;
    }

    TiXmlDocument doc; 
    doc.LoadFile(fp);
    TiXmlHandle docHandle(&doc);
    TiXmlHandle root = docHandle.FirstChildElement( "redis_conf" );
    if (!root.Element()) {
        fclose(fp);
        return false;
    }

    TiXmlElement* serverElement = root.FirstChildElement("server").Element();
    while (serverElement) {
        RedisConf conf;
        conf._host = serverElement->Attribute("ip");
        conf._port = atoi(serverElement->Attribute("port"));
        conf._passwd = serverElement->Attribute("passwd");
        uint32_t timeout = atoi(serverElement->Attribute("timeout"));
        struct timeval tv = {timeout, 0}; 
        conf._timeout = tv;

        confArray.push_back(conf);
        serverElement = serverElement->NextSiblingElement("server");
    }    

    fclose(fp);

    return !confArray.empty();
}

void Cache::checkRedis()
{
    for (uint32_t i = 0; i < _clientPool.size(); i++) {
		RedisClient* client = _clientPool[i];
        if (!client->ping()) {
            client->connect();
        }
    }
}

bool Cache::get(const string& key, string& value)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->get(key, value);
    return (ret == kStatusOK);
}

bool Cache::set(const string& key, const string& value, uint32_t duration)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->set(key, value, duration);
    return (ret == kStatusOK);
}

bool Cache::setNX(const string& key, const string& value)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->set(key, value, 0, 2);
    return (ret == kStatusOK);
}

bool Cache::remove(const string& key)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->remove(key);
    return (ret == kStatusOK);
}

bool Cache::incrBy(const string& key, const int64_t increment, int64_t& rvalue)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->incrBy(key, increment, rvalue);
    return (ret == kStatusOK);
}

bool Cache::enqueue(const string& topic, const std::vector<string>& vals)
{
    RedisClient* client = dispatch(topic);
    if (!client) {
        return false;
    }
    int ret = client->rpush(topic, vals);
    return (ret == kStatusOK);
}

bool Cache::enqueue(const string& topic, const string& value)
{
    RedisClient* client = dispatch(topic);
    if (!client) {
        return false;
    }
    std::vector<string> v;
    v.push_back(value);
    int ret = client->rpush(topic, v);
    return (ret == kStatusOK);
}

bool Cache::dequeue(const string& topic, string& value)
{
    RedisClient* client = dispatch(topic);
    if (!client) {
        return false;
    }
    int ret = client->lpop(topic, value);
    return (ret == kStatusOK);
}

bool Cache::dequeueAll(const string& key, vector<string>& members)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->lrange(key, 0, -1, members);
    if (ret == kStatusOK) {
        return remove(key);
    }
    return false;
}

bool Cache::addToSet(const string& key, const string& value)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->sadd(key, value);
    return (ret == kStatusOK);
}

bool Cache::getMembers(const string& key, vector<string>& members)
{
    RedisClient* client = dispatch(key);
    if (!client) {
        return false;
    }
    int ret = client->smembers(key, members);
    return (ret == kStatusOK); 
}
