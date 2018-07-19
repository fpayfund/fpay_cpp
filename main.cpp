#include <getopt.h>
#include <string>
#include <map>
#include <unistd.h>

#include "core/corelib/MfcAppContext.h"
#include "core/corelib/BackLinkHandler.h"
#include "core/corelib/InnerConn.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "core/corelib/WrapServerStart.h"
#include "core/corelib/WriterImp.h"

#include "server/FPayServer.h"
#include "server/FPayServerCore.h"
#include "client/FPayClientCore.h"

#include "flags.h"

using namespace core;
using namespace std;

DEFINE_int(port, 9527, "pay listen port")
DEFINE_string(address,"","local address")
DEFINE_string(public_key,"","local public key")
DEFINE_string(public_key,"","local_private_key")

int main(int argc, char* argv[])
{


	WrapServerStart::init();
	//服务端模块初始化
    MfcAppcontext __server_appContext; 
	FPayServerCore* __server_core = FPayServerCore::Singleton();	
	InnerConnCreator __server_screator;
	BackLinkHandler __server_handler; 
	FPayServer __fpay_server; 
	vector<uint16_t> ports;
	ports.push_back(FLAG_port);
	FPayServer.setExpectPorts(ports);

	__server_core.setServerConnCreator(&__server_screator);
	__server_core.setClientConnCreator(&__server_screator); 
									
	__server_handler.setAppContext(&__server_appContext);
	__fpay_server.setConnManager(&__server_core); 
	__fpay_server.setLinkHandler(&__server_handler);
	__fpay_server.setLinkEvent(&__server_core);
								      																			
	__server_appContext.addEntry(FPayServerCore::getFormEntries(),__server_core, __server_core);
	
	__fpay_server.init();
    //客户端模块初始化
	MfcAppcontext __client_appContext;	
	InnerConnCreator client_ccreator; 
	BackLinkHandler __client_handler; 
	__client_handler.setAppContext(&__client_appContext); 
	FPayClientCore* __fpay_client = FPayClientCore::Singleton(); 
	__fpay_client.setClientConnCreator(&client_ccreator); 
	__fpay_client.setLinkHandler(&__client_handler); 
	__client_appContext.addEntry(FPayClientCore::getFormEntries(),__fpay_client,__fpay_client);
    
	__fpay_client.init();

	WrapServerStart::run();
}


