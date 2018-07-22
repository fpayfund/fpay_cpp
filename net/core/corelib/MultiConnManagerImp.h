#ifndef MULTICONNMANAGERIMP_H
#define MULTICONNMANAGERIMP_H
#include "ConnManagerLinkImp.h"
#include "core/sox/snox.h"
namespace core{
	class MultiConnManagerImp: public LazyDelConnManager{
	protected:
		uint32_t cid;

		typedef std::map<uint32_t, IConn *> connect_t;

		connect_t connects;
	public:
		MultiConnManagerImp();
		virtual ~MultiConnManagerImp();
		virtual void eraseConnect(IConn *conn);
		virtual void eraseConnectById(uint32_t id);

		virtual bool dispatchById(uint32_t cid, Sender &request);
		virtual bool dispatchByIds(const std::set<uint32_t> &ids, Sender &request, uint32_t exp = NONEEXP_CID);

		virtual IConn *getConnectById(uint32_t id);

		size_t getConnSize() {
			return connects.size();
		}

		virtual void onConnCreate(IConn *conn);
	};

}
#endif

