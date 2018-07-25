#ifndef __FPAY_SERVER_H_
#define __FPAY_SERVER_H_

#include "common/core/ibase.h"
#include "common/core/ilink.h"
#include "common/core/sender.h"
#include "core/sox/tcpsock.h"
#include "core/corelib/MAbstractServer.h"

class FPayServer;

class ServerSocketHelper:public sox::ServerSocket
{
	protected:
		FPayServer *imp;
	public:
		ServerSocketHelper(FPayServer *, SOCKET so);
		virtual ~ServerSocketHelper(){}
		virtual void onAccept(SOCKET so, u_long ip, int port);
		void start();
};

class FPayServer: public core::MAbstractServer
{
	protected:
		std::vector<ServerSocketHelper *> helpers;
		friend class ServerSocketHelper;
		std::vector<uint16_t> expectPorts;
		std::vector<uint16_t> curports;
		std::string ip;
	public:
		FPayServer();
		~FPayServer();

		void onAccept(SOCKET so, u_long ip, int port);
		virtual ServerSocketHelper *createHelper(const char *ip);
		virtual std::string getIp();
		virtual std::vector<uint16_t> getPorts();

		virtual void startSV();
		inline void setExpectPorts(const std::vector<uint16_t>& ports)
		{
			expectPorts = ports;
		}
	protected:
		virtual ServerSocketHelper *create(const char* ip, uint16_t p, uint16_t &cur);
};

#endif
