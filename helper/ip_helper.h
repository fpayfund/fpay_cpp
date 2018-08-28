#ifndef __FPAY_IP_HELPER_H_
#define __FPAY_IP_HELPER_H_
#include <string>
#include <map>

//获取所有接口的ip信息
void GetAllIPInfos(std::map<std::string,std::string>& eth2ip);

//获取对应接口的ip，比如eth1
std::string GetIPByEthName(const std::string& eth);

#endif

