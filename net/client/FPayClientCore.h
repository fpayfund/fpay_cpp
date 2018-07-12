#ifndef FPAY_CLIENT_CORE_H_
#define FPAY_CLIENT_CORE_H_
#include "common/core/iserver.h"
#include "common/core/ibase.h"
#include "server_common/protocol/daemon.h"
#include "common/protocol/prouter.h"
#include "server_common/protocol/prouter.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "server_common/helper/TimerHandler.h"
#include "core/sox/mutex.h"
#include <time.h>
#include <deque>

#include "IClientCallbackIf.h"
#include "IClientTimerIf.h"
#include "IClientSendIf.h"

using namespace std;

class FPayClientCore: 
	public core::PHClass, 
	public core::IFormTarget, 
	public core::ILinkHandlerAware,
	public core::MultiConnManagerImp,
	public IClientSendIf
{
	typedef struct _parent_info
	{
		uint32_t cid;          
		Byte32 address;    

		time_t last_ping_time; 
		uint64_t pay_comfirm_count;   
		_parent_info()
		{
			cid = 0;

			last_ping_time = time(NULL);
			pay_comfirm_count = 0;
		}
		_parent_info(uint32_t c, Byte32 addr):
			cid(c),
			address(addr),
			last_ping_time(time(NULL),
			pay_comfirm_count(0)
		{
		}
	}parent_info_t;

public:
    FPayClientCore(IClientCallbackIf* cif,IClientTimerIf* tif);
	virtual ~FPayClientCore();

	//初始启动，输入初始启动的节点IP,PORT
	void startSV(const string& ip,uint16_t & port);
	
	DECLARE_FORM_MAP

	//收到同步区块的回应
	void onSyncBlocksRes(SyncBlocksRes* sync_res, core::IConn* c);
	//收到获取邻居节点的回应
	void onGetRelativesRes(GetRelativesRes* rela_res,core::IConn* c);
	//收到支付请求的回应
	void onPayRes(PayRes* pay_res, core::IConn* c);
	//收到节点注册的回应
	void onNodeRegisterRes( NodeRegisterRes* reg_res, IConn* c);
	//收到下发的区块广播
	void onBlockBroadcast(BlockBroadcast* broadcast, IConn* c);
	

    //virtual void onConnected(core::IConn* c);//这个事件不一定有，
	//如果connect 立马连接成功，就不会存在这个事件，故不能随便用这个事件
    virtual void eraseConnect(core::IConn *conn);
    virtual void onError(int ev, const char *msg, core::IConn *conn);

private:
    core::IConn *connectNode(const string& ip, uint16_t port);
     
    bool linkCheck(); 
    bool ping();



	//父节点列表信息
	map<uint32_t,parent_info_t> parent_infos 
    //父节点地址到连接的映射
	map<Byte32,/*node address*/ uint32_t/*conn id*/,compByte32> address_2_connid;
	
	//定时器
    TimerHandler<FPayClientCore, &FPayClientCore::linkCheck> timer_link_check; 
    TimerHandler<FPayClientCore, &FPayClientCore::ping> timer_ping;

	//门面回调
	IClientCallbackIf* net_proxy;
	IClientTimerIf* timer_proxy;
};

#endif

