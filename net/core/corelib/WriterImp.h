#ifndef CORE_ABSTRACTWRITER_H_
#define CORE_ABSTRACTWRITER_H_
#include "common/core/ibase.h"
#include "common/core/iserver.h"
#include <list>
#include <vector>
#include "RouterBase.h"

namespace core{

	class StopPacket: public IPacketBase{
	public:
		virtual int emit(core::IConn *conn, core::IServerIdDispatcher* ){
			return -1;
		}

		virtual void gc(IPacketGc *){
			//do nothing
		}
	};
class AbstractWriter: public IWriter
{
protected:
	std::vector<IPacketBase *> sendBuf;
	StopPacket stopPacket;

public:
	~AbstractWriter();
	
	virtual void stop();
	virtual void send(IPacketBase *);
};

}
#endif /*WRAPSERVERSOCKET_H_*/

