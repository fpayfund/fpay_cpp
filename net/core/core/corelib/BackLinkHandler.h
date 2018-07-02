#ifndef BACKLINKHANDLER_H_
#define BACKLINKHANDLER_H_

#include <core/sox/snox.h>
#include <common/core/ilink.h>
#include <common/core/iserver.h>
#include "AStatLinkHandler.h"
namespace core {
class BackLinkHandler : public AStatLinkHandler,
						public IAppContextAware,
						public sox::Handler
{
	uint32_t m_uProcSize;

public:

	BackLinkHandler();
	virtual ~BackLinkHandler();

	virtual int onData(const char*, size_t, IConn *conn, int type=0);

	virtual int doRequest(Request &request, IConn *rconn);

	virtual void handle(int sig);
};
}
#endif /*BACKLINKHANDLER_H_*/

