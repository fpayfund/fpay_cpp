#ifndef DOUBLELINK_INC_H_
#define DOUBLELINK_INC_H_
#include "DoubleLinkServer.h"
#include "DoubleLinkClient.h"
#include "core/corelib/MfcAppContext.h"
#include "core/corelib/BackLinkHandler.h"
//#include "core/corelib/TServerConnCreator.h"
#include "core/corelib/InnerConn.h"
#include "core/corelib/MultiConnManagerImp.h"

#include "core/corelib/WrapServerStart.h"
#include "core/corelib/WriterImp.h"


using namespace core;
using namespace server::config;
using namespace doublelink;
	//
#define CONFIG_DOUBLELINK_SERVER_INIT( proxy_layer, broadcast, unicast, ports, deamon_regist_name) \
    MfcAppcontext __doublelink_appContext; \
	DoubleLinkRouter __doublelink_router(proxy_layer,broadcast,unicast);	\
	InnerConnCreator doublelink_screator;	\
	InnerConnCreator doublelink_ccreator; \
	BackLinkHandler __doublelink_handler; \
	DoubleLinkServer __doublelink_server; \
	__doublelink_router.setServerConnCreator(&doublelink_screator); \
	__doublelink_router.setClientConnCreator(&doublelink_ccreator); \
	\
	__doublelink_handler.setAppContext(&__doublelink_appContext); \
	__doublelink_server.setConnManager(&__doublelink_router); \
	__doublelink_server.setLinkHandler(&__doublelink_handler);  \
	__doublelink_server.setLinkEvent(&__doublelink_router); \
        DaemonClient __extClientDaemon;    \
        __doublelink_router.setServerIdDispatcher(&__extClientDaemon);\
        __doublelink_server.setDaemonClient(&__extClientDaemon);\
        __doublelink_server.setServerIdDispatcher(&__extClientDaemon);\
        __doublelink_server.setName(deamon_regist_name); \
        __extClientDaemon.setClientConnCreator(&doublelink_ccreator);\
        __extClientDaemon.setLinkHandler(&__doublelink_handler);      \
        __extClientDaemon.setProcStat(&__doublelink_handler); \
        __extClientDaemon.setDaemonConnectWatcher(&__doublelink_server); \
        __extClientDaemon.setServer(&__doublelink_server); \
       __doublelink_appContext.addEntry(DaemonClient::getFormEntries(),    &__extClientDaemon, &__extClientDaemon); \
       __doublelink_appContext.addEntry(DoubleLinkRouter::getFormEntries(),&__doublelink_router,&__doublelink_router);\
       __doublelink_appContext.addEntry(DoubleLinkServer::getFormEntries(),&__doublelink_server,&__doublelink_server);\
       __doublelink_server.setExpectPorts(ports);


#define DOUBLELINK_SERVER_START __extClientDaemon.startSV(); __doublelink_server.startSV();


#define CONFIG_DOUBLELINK_CLIENT_INIT() \
	\
	MfcAppcontext __doublelink_appContext;	\
	InnerConnCreator doublelink_screator;	\
	InnerConnCreator doublelink_ccreator; \
	BackLinkHandler __doublelink_handler; \
	__doublelink_handler.setAppContext(&__doublelink_appContext); \
	DoubleLinkClient __doublelink_client; \
	__doublelink_client.setServerConnCreator(&doublelink_screator); \
	__doublelink_client.setClientConnCreator(&doublelink_ccreator); \
	__doublelink_client.setLinkHandler(&__doublelink_handler); \
    __doublelink_appContext.addEntry(DoubleLinkClient::getFormEntries(),&__doublelink_client,&__doublelink_client);\
 

#define DOUBLELINK_CLIENT_START(proxys) __doublelink_client.startSV(proxys);

#endif /*DOUBLELINK_INC_H_*/

