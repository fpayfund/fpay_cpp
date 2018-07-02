#ifndef ABSTRACTCONN_H_
#define ABSTRACTCONN_H_
#include "common/core/ilink.h"
#include "common/int_types.h"
#include <map>

namespace core{
	struct LinkPacker;
}

namespace sox{
	class PackBuffer;
}
namespace core {
class AbstractConn : public IConn {
protected:
	uint16_t serial;

	int tmo;
	
	
	uint16_t nextSerial();


public:
	AbstractConn();
	~AbstractConn();
	
	virtual void send(Sender &resp);

	virtual void setTimeout(int tm);

	virtual void *getData();

	virtual void notifyErr(int err, const char *msg);
	//function send = 0 
};
}

#endif /*ABSTRACTCONN_H_*/
