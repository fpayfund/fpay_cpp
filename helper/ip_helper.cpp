#include "ip_helper.h"
#include <string>
#include <map>
#include <stdio.h>      
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h> 
#include <string.h> 
#include <arpa/inet.h>

using namespace std;

void GetAllIPInfos(map<string,string>& eth2ip)
{	
	struct ifaddrs * ifAddrStruct = NULL;
	void * tmpAddrPtr = NULL;

	getifaddrs(&ifAddrStruct);

	while (ifAddrStruct!=NULL) {
		if (ifAddrStruct->ifa_addr->sa_family == AF_INET) { 
			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);

			eth2ip[ifAddrStruct->ifa_name] = addressBuffer;
		} else if (ifAddrStruct->ifa_addr->sa_family==AF_INET6) { 

			tmpAddrPtr=&((struct sockaddr_in *)ifAddrStruct->ifa_addr)->sin_addr;
			char addressBuffer[INET6_ADDRSTRLEN];
			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);

			eth2ip[ifAddrStruct->ifa_name] = addressBuffer;
		} 
		ifAddrStruct=ifAddrStruct->ifa_next;
	}
}


string GetIPByEthName(const string& eth)
{
	map<string,string> eth2ip;
	GetAllIPInfos(eth2ip);
	string ip;
	map<string,string>::iterator it;
	if( (it = eth2ip.find(eth)) != eth2ip.end() ) {
		ip = it->second;
	}
	return ip;
}

/*
int main(int argc,char* argv[])
{
	map<string,string> eth2ip;
	GetAllIPInfos(eth2ip);
	map<string,string>::iterator it;
	for( it = eth2ip.begin(); it != eth2ip.end(); it++ )
	{
		printf("%s IP Address %s\n", it->first.c_str(),it->second.c_str());
	}

	string ip = GetIPByEthName("eth1");

	fprintf(stderr,"eth1 ip is %s\n",ip.c_str());
}*/
