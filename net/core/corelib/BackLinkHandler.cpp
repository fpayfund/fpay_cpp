#include "BackLinkHandler.h"
#include "common/core/ibase.h"
#include "core/sox/logger.h"
#include "WrapForwardBuffer.h"
using namespace core;

#define REPORT_TIME_OUT 10 * 1000

//wuji start
#include <stdio.h>
//wuji end


BackLinkHandler::BackLinkHandler()
{
    select_timeout(REPORT_TIME_OUT);
    m_uProcSize = 0;

}

BackLinkHandler::~BackLinkHandler()
{
}

int BackLinkHandler::onData(const char* data, size_t size, IConn *conn,
        int type) {
    WrapForwardBuffer fb(data, size);

    while (!fb.empty()) {
        if (fb.size() < 4)
            break; // need more
        uint32_t length = Request::peeklen(fb.data()); // 察看整个包的长度
        if(length > getPackLimit()){
            log(Info, "[BackLinkHandler::onData]: data is too long!  length=%ld,  Limit=%ld", length, getPackLimit());
            return -1;
        }

        if (fb.size() < length)
            break; // need more
        
        Request request(fb.data(), length);
        request.head();
        //log(Debug, "receive SID:", request.getSid());

        request.setConnType(type);

        incProc(length);

        if(doRequest(request, conn) == -1)
            return -1;

        fb.erase(length);
    }
    return (int)fb.offset();
}

int BackLinkHandler::doRequest(Request &request, IConn *conn){
    m_uProcSize ++;

    IWriter *writer = appContext->requestDispatch(request, conn);
    if(writer){
        return writer->flush(conn);
    }else{
        return 0;
    }
}

void BackLinkHandler::handle(int sig)
{
    log(Notice, "proc:%u", m_uProcSize);
#ifdef DYNAMIC_LOG
    if (m_uProcSize > 5000)
    {
        LogLevelNoticeSigHandler(1);
    }else if (m_uProcSize>0){
        LogLevelDebugSigHandler(1);
    }
#endif

    m_uProcSize = 0;
    select_timeout(REPORT_TIME_OUT);
}
