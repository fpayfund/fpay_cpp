//参数：

//ipbuf ：存放得到的IP地址列表的数组，实际大小由bufcount指定。

//bufcount:想要得到的IP地址列表的个数。
#include<stdio.h>
#include<string.h>
#ifdef _WIN32

#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib,"ws2_32.lib")

#else

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
//#include <net/if_arp.h>
#include <arpa/inet.h>
#include <unistd.h>

#endif

#include "selfip.h"

std::vector<uint32_t> getselfiplist() {
	int bufcount = 10;
	std::vector<uint32_t> ret;
	int i;
#ifdef _WIN32
	int count = 0;
	char hostname[128];
	struct hostent* inaddrs;
	if(gethostname(hostname,128)==0)
	{
		inaddrs=gethostbyname(hostname);
		if(inaddrs)
		{
			count=inaddrs->h_length/sizeof(in_addr);
			if(count>bufcount)count=bufcount;
			for(i=0;i<count;i++)
			{
				ret.push_back(*(unsigned long*)inaddrs->h_addr_list[i]);
			}
		}
	}
#else
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock>=0)//!<0
	{
		for (i=0; i<bufcount; i++) {
			sprintf(ifr.ifr_name, "eth%d", i);
			if (ioctl(sock, SIOCGIFADDR, &ifr)<0)
				break;
			memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
			ret.push_back(sin.sin_addr.s_addr);
		}
		close(sock);
	}
#endif
	return ret;
}
