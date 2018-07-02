#ifndef DOUBLELINK_DEFINE_H_
#define DOUBLELINK_DEFINE_H_
#include <string>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

namespace doublelink {

typedef      std::string   PROXY_INFO_T;
typedef      std::string   LOCAL_INFO_T;
typedef      std::string   IP_ADDR_T;
typedef      int32_t       PORT_T;

static const PROXY_INFO_T RANDOM_DISPATCH_PROXY = "0.0.0.0:0";
static inline  PROXY_INFO_T format_proxy_info( const std::string& ip, int32_t port )
{
    char buf[20];
    sprintf(buf, "%d", port);
    return (ip + ":" + buf );
}

static inline IP_ADDR_T get_ip_from_proxy_info( const PROXY_INFO_T& proxy_info )
{
    int npos = proxy_info.find(':',0);
	return proxy_info.substr(0,npos);
}

static inline PORT_T get_port_from_proxy_info( const PROXY_INFO_T& proxy_info )
{
    int npos = proxy_info.find(':',0);
	return atoi(proxy_info.substr(npos+1,-1).c_str());
}

static inline bool DoubleLink_PraseSessionInfo(const std::string& json,std::string& session,uint8_t& operate)
{
    if( json.empty() ) return false;
    size_t pos = json.find("|");
    if( pos == std::string::npos ) return false;
    session = json.substr(0,pos);
    
    size_t pos1 = json.find('|',pos+1);
    if( pos1 == std::string::npos )
    {
        atoi((json.substr(pos+1)).c_str());
    }
    else
    {
       atoi((json.substr(pos+1,pos1-pos-1)).c_str());
    } 
    return true;
}

static inline std::string DoubleLink_FromatSessionInfo(const std::string& session,uint8_t operate)
{
    char buf[200];
    sprintf(buf,"%s|%u",session.c_str(),operate); 
    return buf;
}

}

#endif

