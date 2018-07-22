#include "RedisProtocol.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

namespace core {
using namespace std;

//static char REPLY_TYPE_DATA_BLOCK = '$';
//static char REPLY_TYPE_INT        = ':';


string format_int_to_string(uint32_t i)
{
  char buf[20];
  sprintf(buf,"%u",i);
  return string(buf);
}

int charArrayToInt(const char* data,size_t sz)
{
    char ibuf[20] = {0};
    int len = sz > 19 ? 19 : sz;
    memcpy(ibuf,data,len);
    ibuf[len] = '\0';
    return atoi(ibuf);
}

int RedisProtocol::findCRLF(const char* data, size_t sz)
{
    if(sz == 0 ) return 0;
    const char* p = NULL;
    size_t i = 0;
    bool find = false;
    while( i < sz-1 )
    {
        p = data + i;
        if( *p == '\r' )
        {
            if(*(p+1) == '\n')
            {
                find = true;
                break;
            }
            else
            {
                return -1;
            }
        }
        i++;
    }
    if(!find) return 0;
    return i;
}


void RedisProtocol::ConstructSubscribe(const std::string& key, std::string& packet)
{
    packet = string("*2\r\n$9\r\nSUBSCRIBE\r\n") + string("$")
         + format_int_to_string(key.size()) + "\r\n" + key + "\r\n";
}

void RedisProtocol::ConstructPublish(const std::string& key, const std::string& value, std::string& packet )
{
    packet = string("*3\r\n$7\r\nPUBLISH\r\n") + string("$") 
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(value.size()) + "\r\n" + value + "\r\n";
}

void RedisProtocol::ConstructHset(const std::string& key, const std::string& field, const std::string& value, std::string& packet )
{
    packet = string("*4\r\n$4\r\nHSET\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(field.size()) + "\r\n" + field + "\r\n"
             + string("$") + format_int_to_string(value.size()) + "\r\n" + value + "\r\n";
}

void RedisProtocol::ConstructHdel(const std::string& key, const std::string& field, std::string& packet )
{
    packet = string("*3\r\n$4\r\nHDEL\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(field.size()) + "\r\n" + field + "\r\n";
}

void RedisProtocol::ConstructHincrby(const std::string& key, const std::string& field, const std::string& value, std::string& packet )
{
    packet = string("*4\r\n$7\r\nHINCRBY\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(field.size()) + "\r\n" + field + "\r\n"
             + string("$") + format_int_to_string(value.size()) + "\r\n" + value + "\r\n";
}

void RedisProtocol::ConstructHmset(const std::string& key, const std::vector< std::pair<std::string,std::string> >& f2vs, std::string& packet )
{
    uint32_t param_count = 2 + f2vs.size()*2;

    packet = string("*") + format_int_to_string(param_count) + string("\r\n$5\r\nHMSET\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n";
    vector<std::pair<std::string,std::string> >::const_iterator it;
    for( it = f2vs.begin(); it != f2vs.end(); ++it )
    {
        packet += string("$") + format_int_to_string(it->first.size()) + "\r\n" + it->first + "\r\n"
             + string("$") + format_int_to_string(it->second.size()) + "\r\n" + it->second + "\r\n";
    }
}

void RedisProtocol::ConstructSet(const std::string& key, const std::string& value, std::string& packet )
{
    packet = string("*3\r\n$3\r\nSET\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(value.size()) + "\r\n" + value + "\r\n";
}


void RedisProtocol::ConstructSadd(const std::string& key, const std::string& value, std::string& packet )
{
    packet = string("*3\r\n$4\r\nSADD\r\n") + string("$")
             + format_int_to_string(key.size()) + "\r\n" + key + "\r\n"
             + string("$") + format_int_to_string(value.size()) + "\r\n" + value + "\r\n";
}

int RedisProtocol::parseInt(const char* data, size_t sz, int& _ret)
{
    int r = findCRLF(data,sz);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    _ret = charArrayToInt(data,r);
   return r+2;
}

int RedisProtocol::parseString(const char* data, size_t sz, std::string& _out)
{
    int r = findCRLF(data,sz);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    _out.assign(data,r);
    return r+2;
}


int RedisProtocol::ParseSubscribeResponse(const char* data, size_t sz)
{
    int block_count = 0;
    int pos = 0;
    int r = ParseBlocksHead(data,sz,block_count);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;
    string block;
    r = ParseBlock(data+pos, sz-pos, block);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;
    r = ParseBlock(data+pos, sz-pos, block);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;
    if( *(data+pos) != ':' ) return -1;
    int ret = 0;
    r = parseInt(data+pos+1,sz-pos-1,ret);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    if( ret != 1 )
    {
       return -1;
    }
    pos += r+1;
    return pos;
}


int RedisProtocol::ParseBlocksHead(const char* data, size_t sz, int& _block_count )
{
    if(*data != '*') return -1;
    int r = parseInt(data+1,sz-1,_block_count);
    
    if( r == 0 )
    { 
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    return r+1; 
}


int RedisProtocol::parseBlockLen(const char* data, size_t sz, int& _len)
{
    if( sz == 0 ) return 0;
    if(*data != '$') return -1;
    int len = 0;
    const char* pos = data + 1;
    size_t size = sz - 1;
    int r = parseInt(pos,size,len);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    _len = len;
    return r+1;
}

int RedisProtocol::parseBlockData(const char* data, size_t sz, string& _data)
{
    int r = parseString(data,sz,_data);
    return r;
}


int RedisProtocol::ParseBlock(const char* data, size_t sz, string& _block)
{
    int pos = 0;
    int len = 0;
    int r = parseBlockLen(data,sz,len);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;
    r = parseBlockData( data+pos, sz-pos, _block );
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;
     
    return pos;
 }

char RedisProtocol::checkReplyType(const char* data)
{
    return *data;
}

int RedisProtocol::ParseSubscribeMessage( const char* data, size_t sz, std::string& key, std::string& value )
{
    int block_count = 0;
    int pos = 0;
    int r = 0;
    r = ParseBlocksHead(data,sz,block_count);
    
    if( r == 0 )
    {
        return 0 ;
    }
    else if( r == -1 )
    {  
        return -1;
    }
    pos += r;
    
    if( block_count != 3 ) return -1; 
   
     
    string block;
    r = ParseBlock(data+pos, sz-pos, block);
    
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    pos += r;   

    if( block != "message" && block != "MESSAGE" )  return -1;
    
    r = ParseBlock(data+pos, sz-pos, key);
   
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }    
    pos += r;
    
    r = ParseBlock(data+pos, sz-pos, value);
    
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }

    pos += r;    
    return pos;
}


int RedisProtocol::ParseIntResponse(const char* data, size_t sz, int& _ret )
{
    int r = 0;
    if( *(data) != ':' ) return -1;
    r = parseInt(data+1,sz-1,_ret);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    return r+1;
}


int RedisProtocol::ParseOkResponse(const char* data, size_t sz, std::string& out)
{
    int r = 0;
    if( *(data) != '+' ) return -1;
    r = parseString(data+1,sz-1,out);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    return r+1;    
}

int RedisProtocol::ParseFailResponse(const char* data, size_t sz, std::string& err)
{
    int r = 0;
    if( *(data) != '-' ) return -1;
    r = parseString(data+1,sz-1,err);
    if( r == 0 )
    {
        return 0;
    }
    else if( r == -1 )
    {
        return -1;
    }
    return r+1;
}

char RedisProtocol::ReturnType(const char* data)
{
    return *data;
}

}

