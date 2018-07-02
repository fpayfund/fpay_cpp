#include "common/core/ilink.h"
namespace core{
	template<class C> class TServerConnCreator: public IServerConnCreator{
	public:
		TServerConnCreator(){}
		virtual IConn *creatConnect(SOCKET so, uint32_t ip, int port, ILinkHandler *ih, ILinkEvent *ie, CreateCallback *cb){
			C *conn = new C(so, (u_long)ip, port, ih, ie);
			conn->setHandler(ih);
			conn->setLinkEvent(ie);
			if(cb)
				cb->onConnCreate(conn);
			conn->select(0, sox::SEL_READ);
			return conn;
		}
	};
}

