#include <string>
#include "PayConfig.h"
#include "Cache.h"
#include "ecc_helper.h"
#include "fpay_protocol.h"
#include "PayClient.h"
#include "core/corelib/MfcAppContext.h"
#include "core/corelib/BackLinkHandler.h"
#include "core/corelib/InnerConn.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "core/corelib/WrapServerStart.h"
#include "core/corelib/WriterImp.h"
#include "core/sox/selsel.h"
#include "server/FPayServer.h"
#include "server/FPayServerCore.h"
#include "client/FPayClientCore.h"
#include "server/FPayConfig.h"

#include "helper/ecc_helper.h"
#include "flags.h"

using namespace sox;
using namespace core;
using namespace fpay::protocol;
using namespace sox;
using namespace std;

DEFINE_string(parent_ip,"","reigster to net ip, if it's NULL,self root")
DEFINE_int(parent_port,9527,"register to net port")
DEFINE_string(cfg_file,"pay.xml","local node config file path")

int main(int argc, char* argv[])
{
	//获取命令行参数
	if( FlagList::SetFlagsFromCommandLine(&argc,argv,false) != 0 ){
		//fprintf(stderr,"parse comman line failed\n");
		FlagList::Print(NULL,false);
		return -1;
	}

	if( FLAG_help ) {
		FlagList::Print(NULL,false);
		return 0;
	}


	if( PayConfig::getInstance()->Load(FLAY_cfg_file) == false ) {

		fprintf(stderr, "load pay.xml config failed\n");
		return 1;
	}
	 //网络事件循环初始化
	WrapServerStart::init();
	
	//客户端模块初始化
	MfcAppcontext __client_appContext;	
	InnerConnCreator client_ccreator; 
	BackLinkHandler __client_handler; 
	__client_handler.setAppContext(&__client_appContext); 
	PayClient* __fpay_client = PayClient::getInstance(); 
	__fpay_client->setClientConnCreator(&client_ccreator); 
	__fpay_client->setLinkHandler(&__client_handler); 
	__client_appContext.addEntry(FPayClientCore::getFormEntries(),__fpay_client,__fpay_client);
    
	__fpay_client->init(local_address,local_public_key,local_private_key);

    vector< pair<string,uint16_t> > parent_nodes;
	if( !FLAG_parent_ip.empty() ) {
		parent_nodes.push_back(make_pair(FLAG_parent_ip,FLAG_parent_port));
	}
	__fpay_client->start(parent_nodes);

	WrapServerStart::run();
	
	return 0;
}
