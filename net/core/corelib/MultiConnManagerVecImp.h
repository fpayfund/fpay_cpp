#ifndef MULTICONNMANAGERVECIMP_H
#define MULTICONNMANAGERVECIMP_H
#include "ConnManagerLinkImp.h"
#include <list>
#include "core/sox/snox.h"
namespace core{
	class MultiConnManagerVecImp: public LazyDelConnManager{
	protected:
		uint32_t serialId;
		typedef std::vector<IConn *> connect_t;

		connect_t connects;
		std::list<uint32_t> freeList;
	public:
		MultiConnManagerVecImp();
		virtual ~MultiConnManagerVecImp();
		virtual void eraseConnect(IConn *conn);
		virtual void eraseConnectById(uint32_t id);

		virtual bool dispatchById(uint32_t cid, Sender &request);
		virtual bool dispatchByIds(const std::set<uint32_t> &ids, Sender &resp, uint32_t exp = NONEEXP_CID);

		virtual IConn *getConnectById(uint32_t id);

		size_t getConnSize() {
			return connects.size();
		}


		virtual void onConnCreate(IConn *conn);

	};

}
#endif

