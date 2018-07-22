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

#include "helper/ecc_helper.h"
#include "flags.h"

using namespace core;
using namespace std;

DEFINE_string(broadcast_ip,"","block broadcast listent ip")
DEFINE_int(broadcat_port,9828,"block broadcast listent port")
DEFINE_int(pay_port, 9527, "pay listen port")
DEFINE_string(local_address_file,"../conf/address.txt","local node address file,base58 encode")
DEFINE_string(public_key_file,"../conf/pubkey.txt","local node public key file,base58 encode")
DEFINE_string(private_key_file,"../conf/privkey.txt","local nide private key file,base58 encode")

//读取文件数据（地址，公钥，私钥）
string ReadFile( const string& file ) {
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

	//获取命令行参数
	if( SetFlagsFromCommandLine(argc,argv,false) != 0 ){
		fprintf(stderr,"parse comman line failed\n");
		return -1;
	}

	string address = ReadFile(FLAG_local_address_file);
    //将Base58地址转换为二进制地址
    Byte20 local_address;	
    Base58AddressToBin(address,local_address.u8);

	string public_key = ReadFile(FLAG_local_public_key_file);
    //将BASE58的公钥转换为二进制公钥
	Byte32 local_public_key;
	uint32_t KeyFromBase58(public_key,local_public_key.u8);


	string private_key = ReadFile(FLAG_local_private_key_file);
    //将BASE58的私钥转换为二进制公钥
	Byte32 local_private_key;
	uint32_t KeyFromBase58(public_key,local_private_key.u8);

    //网络事件循环初始化
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
	
	__fpay_server.init(local_address,local_public_key,local_private_key);
    
	//客户端模块初始化
	MfcAppcontext __client_appContext;	
	InnerConnCreator client_ccreator; 
	BackLinkHandler __client_handler; 
	__client_handler.setAppContext(&__client_appContext); 
	FPayClientCore* __fpay_client = FPayClientCore::Singleton(); 
	__fpay_client.setClientConnCreator(&client_ccreator); 
	__fpay_client.setLinkHandler(&__client_handler); 
	__client_appContext.addEntry(FPayClientCore::getFormEntries(),__fpay_client,__fpay_client);
    
	__fpay_client.init(local_address,local_public_key,local_private_key);

	WrapServerStart::run();
}


