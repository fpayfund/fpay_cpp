#ifndef __FPAY_PAY_CLIENT_H_
#define __FPAY_PAY_CLIENT_H_
#include "common/core/iserver.h"
#include "common/core/ibase.h"
#include "common/core/ilink.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "helper/TimerHandler.h"
#include "core/sox/mutex.h"
#include "protocol/fpay_protocol.h"
#include "ClientConst.h"
#include <set>
#include <map>
#include <string>
#include <vector>

using namespace std;
using namespace fpay::protocol;

//单例模式，上层模块可以直接调用接口
class PayClient: 
	public core::PHClass, 
	public core::IFormTarget, 
	public core::ILinkHandlerAware,
	public core::MultiConnManagerImp
{


	//已经连上的上行节点信息
	typedef struct _up_conn_info
	{
		uint32_t cid;      
        Byte20 address;
		string ip;
		uint16_t port;
		time_t last_ping_time; 
		uint64_t pay_count;   
		_up_conn_info()
		{
			cid = 0;
	        port = 0;
			last_ping_time = time(NULL);
			pay_count = 0;
		}
	}up_conn_info_t;

public:

	static PayClient* getInstance()
	{
		if (_instance == NULL) {
			_instance = new PayClient();
		}
		return _instance;
	}

	void init(const Byte20& address,
				const Byte32& public_key,
				const Byte32& private_key);	

	//初始启动，输入初始启动的节点IP,PORT
	void start(const vector< pair<string,uint16_t> >& init_nodes);

DECLARE_FORM_MAP
	
	//收到支付请求的回应
	void onPayRes(PayRes* pay_res, core::IConn* c);
	//收到节点注册的回应
	void onNodeRegisterRes( NodeRegisterRes* reg_res, core::IConn* c);

	void pay();

    //连接抛上来的事件
    virtual void eraseConnect(core::IConn *conn); //连接断开事件
    virtual void onError(int ev, const char *msg, core::IConn *conn); //连接错误


	//连接器
    core::IConn *connectNode(const string& ip, uint16_t port);
   
  
	void send(uint32_t cid, uint32_t uri, const sox::Marshallable& marshal);
	void registerIn(const string& ip, uint16_t port);

	uint32_t findConnByAddress(const Byte20& address);


protected:
				
	PayClient();
	~PayClient();
    static PayClient* _instance;	

	//已经连上的上行节点列表信息
	map<uint32_t,up_conn_info_t> _upConnInfos; 
	//备份的上行节点信息
	set<node_info_t,nodeInfoCmp> _backupNodeInfos;
	//当前父节点地址
    Byte20 _currentParentAddress;
	
    //初始信息
	Byte20 _localAddress;  //本矿工节点地址
	Byte32 _localPublicKey; //本矿工节点的公钥
	Byte32 _localPrivateKey; //本矿工节点的私钥
   
};

#endif

