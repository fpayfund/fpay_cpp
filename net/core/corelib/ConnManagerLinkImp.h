#ifndef CONNMANAGERLINKIMP_H
#define CONNMANAGERLINKIMP_H
#include "common/core/ilink.h"
#include "core/sox/snox.h"
namespace core{
	class ConnManagerLinkImp: public IConnManager, public ILinkEvent{
	public:
		virtual void onConnected(IConn *conn);
		virtual void onClose(IConn *conn);
		//Ö÷¶¯¹Ø±Õ
		virtual void onInitiativeClose(IConn *conn);
		virtual void onError(int ev, const char *msg, IConn *conn);
		virtual void onTimeout(IConn *conn);
	};

	class LazyDelConnManager: public ConnManagerLinkImp, public IDelayDelConn, public sox::Handler{
	protected:
		std::set<uint32_t> toDelete;
	public:
		LazyDelConnManager();

		virtual void handle(int);

		virtual void destroy() { }

		virtual void DelayDelConn(IConn *conn);

		//virtual void DelayDelConnByID(uint32_t cid);

		IConn *createServerConn(SOCKET, uint32_t ip, int port, ILinkHandler *h,
			ILinkEvent *eH);

		IConn *createClientConn(const std::string& ip, uint32_t port,
			ILinkHandler *iH, ILinkEvent *eH);

	};
}
#endif


