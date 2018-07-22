#include  "RedisAsynResponseLinkHandler.h"
#include "RedisProtocol.h"
#include <stdio.h>

namespace core {

using namespace std;

int RedisAsynResponseLinkHandler::onData(const char* data, size_t sz, IConn *conn, int type)
{
    fprintf(stderr,"onData:%s,size:%Zu\n",data,sz);
    size_t real_ret = 0;
    int _ret = 0;
    string _str,_err;
loop:
    int r = 0;
    switch(RedisProtocol::ReturnType(data+real_ret))
    {
        case ':':    
          r = RedisProtocol::ParseIntResponse(data+real_ret, sz-real_ret, _ret);
          break;
        case '+':
          r = RedisProtocol::ParseOkResponse(data+real_ret, sz-real_ret, _str);
          break;
        case '-':
          r = RedisProtocol::ParseFailResponse(data+real_ret, sz-real_ret, _err);
          break;
        default:
          return -1;
    }
    if( r == -1 )
    {
        return -1;
    }
    else if( r == 0 )
    {
        return real_ret;
    }
    real_ret += r;
    
    if( real_ret < sz ) goto loop;
 
    return real_ret; 
}

void RedisAsynResponseLinkHandler::setPackLimit(uint32_t sz)
{
    packet_limit = sz;
}

uint32_t RedisAsynResponseLinkHandler::getPackLimit()
{
    return packet_limit;
}

}
