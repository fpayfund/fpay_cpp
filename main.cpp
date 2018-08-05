#include <getopt.h>
#include <string>
#include <map>
#include <unistd.h>
#include <iostream>
#include <fstream>
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
using namespace std;


DEFINE_int(pay_port, 9527, "pay listen port")
DEFINE_string(cfg_file,"../conf/cfg.xml","local node config file path")
DEFINE_string(address_file,"../conf/address.txt","local node address file,base58 encode")
DEFINE_string(public_key_file,"../conf/pubkey.txt","local node public key file,base58 encode")
DEFINE_string(private_key_file,"../conf/privkey.txt","local nide private key file,base58 encode")
DEFINE_string(parent_ip,"","reigster to net ip, if it's NULL,self root")
DEFINE_int(parent_port,9527,"register to net port")

//读取文件数据（地址，公钥，私钥）
string ReadBase58File( const string& file ) {
    ifstream infile; 
	infile.open(file.data());   //将文件流对象与文件连接起来 
	assert(infile.is_open());   //若失败,则输出错误消息,并终止程序运行 

	string s;
	while(getline(infile,s))
	{
		break;
	}
	infile.close();
	return s;
}


int main(int argc, char* argv[])
{

	int oc;                     /*选项字符 */
	
	while((oc = getopt(argc, argv, "hv")) != -1)
	{
		switch(oc)
		{
			case 'h':
				FlagList::Print(NULL,false);
				return 0;	
			case 'v':
				fprintf(stderr,"0.1.0\n");
				return 0 ;
		}
	}


	//获取命令行参数
	if( FlagList::SetFlagsFromCommandLine(&argc,argv,false) != 0 ){
		fprintf(stderr,"parse comman line failed\n");
		return -1;
	}

	string address = ReadBase58File(FLAG_address_file);
    //将Base58地址转换为二进制地址
    Byte20 local_address;	
    Base58AddressToBin(address,local_address.u8);

	string public_key = ReadBase58File(FLAG_public_key_file);
    //将BASE58的公钥转换为二进制公钥
	Byte32 local_public_key;
    KeyFromBase58(public_key,local_public_key.u8);

	string private_key = ReadBase58File(FLAG_private_key_file);
    //将BASE58的私钥转换为二进制公钥
	Byte32 local_private_key;
	KeyFromBase58(private_key,local_private_key.u8);


	//读取配置文件
	if( !FPayConfig::getInstance()->Load((const char*)FLAG_cfg_file.c_str()) )
	{
		fprintf(stderr,"load config %s failed\n",FLAG_cfg_file.c_str());
		return -1;
	}

    //网络事件循环初始化
	WrapServerStart::init();
	//服务端模块初始化
    MfcAppcontext __server_appContext; 
	FPayServerCore* __server_core = FPayServerCore::getInstance();	
	InnerConnCreator __server_screator;
	BackLinkHandler __server_handler; 
	FPayServer __fpay_server; 
	vector<uint16_t> ports;
	ports.push_back(FLAG_pay_port);
	__fpay_server.setExpectPorts(ports);

	__server_core->setServerConnCreator(&__server_screator);
	__server_core->setClientConnCreator(&__server_screator); 
									
	__server_handler.setAppContext(&__server_appContext);
	__fpay_server.setConnManager(__server_core); 
	__fpay_server.setLinkHandler(&__server_handler);
	__fpay_server.setLinkEvent(__server_core);
								      																			
	__server_appContext.addEntry(FPayServerCore::getFormEntries(),__server_core, __server_core);
	
	__server_core->init(local_address,local_public_key,local_private_key);
	__fpay_server.startSV();

	//客户端模块初始化
	MfcAppcontext __client_appContext;	
	InnerConnCreator client_ccreator; 
	BackLinkHandler __client_handler; 
	__client_handler.setAppContext(&__client_appContext); 
	FPayClientCore* __fpay_client = FPayClientCore::getInstance(); 
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
}


