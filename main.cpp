#include <getopt.h>
#include <string>
#include <map>
#include <unistd.h>


int main(int argc, char* argv[])
{



	WrapServerStart::init();

    MfcAppcontext __server_appContext; 

	//服务端模块初始化
	FPayServerCore* __server_core = FPayServerCore::Singleton();	
	InnerConnCreator __server_screator;
	BackLinkHandler __server_handler; 
	FPayServer __fpay_server; 
	__server_core.setServerConnCreator(&__server_screator);
	__server_core.setClientConnCreator(&__server_screator); 
									
	__server_handler.setAppContext(&__server_appContext);
	__fpay_server.setConnManager(&__server_core); 
	__fpay_server.setLinkHandler(&__server_handler);
	__fpay_server.setLinkEvent(&__server_core);
								      																			
	__server_appContext.addEntry(FPayServerCore::getFormEntries(),__server_core, __server_core);
	
    //客户端模块初始化

    WrapServerStart::run();
}


