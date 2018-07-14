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
	public core::MultiConnManagerImp
{


	//已经连上的上行节点信息
	typedef struct _up_node_info
	{
		uint32_t cid;      
        node_info_t node;
		time_t last_ping_time; 
		uint64_t pay_count;   
		_up_node_info()
		{
			cid = 0;
	        port = 0;
			last_ping_time = time(NULL);
			pay_count = 0;
		}
	}up_node_info_t;

public:
    FPayClientCore(IClientCallbackIf* cif,IClientTimerIf* tif);
	virtual ~FPayClientCore();

	void Init(const Byte32& address,
			  const Byte32& public_key,
			  const Byte32& private_key,
			  uint64_t last_block_idx,
			  const Byte32& last_block_id, 
			  const Byte32& first_root_address);

	//初始启动，输入初始启动的节点IP,PORT
	void startSV(const set<node_info_t,compByte32>& init_nodes);


	//对上层暴露的接口
	//转发支付请求
	int dispatchPayReq( PayReq& pay );


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
	//根节点切换定时器
	bool checkRootSwitch();
	//区块完整性检查定时器
	bool checkBlocksFull();
	//路由优化检查定时器
	bool checkBestRoute();

	//client模块初始化进度标志
	uint64_t init_flag;
	//树的层级，0为根节点
    uint8_t tree_level;

	//已经连上的上行节点列表信息
	map<uint32_t,up_node_info_t> up_node_infos; 
    //已经连上的上行节点地址到连接的映射
	map<Byte32,/*node address*/ uint32_t/*conn id*/,compByte32> address_2_connid;

	//备份的上行节点信息
	set<node_info_t,compByte32> backup_node_infos;
	//当前父节点
	node_info_t current_parent_node;

	//定时器
    TimerHandler<FPayClientCore, &FPayClientCore::linkCheck> timer_link_check; 
    TimerHandler<FPayClientCore, &FPayClientCore::ping> timer_ping;
	TimerHandler<FPayClientCore, &FPayClientCore::checkRootSwitch> timer_check_root_switch;
	TimerHandler<FPayClientCore, &FPayClientCore::checkBlocksFull> timer_check_blocks_full;
	TimerHandler<FPayClientCore, &FPayClientCore::checkBestRoute> timer_check_best_route;
	

    //初始信息
	Byte32 local_address;
	Byte32 local_public_key;
	Byte32 local_private_key;
    uint64_t local_last_block_idx;
	Byte32 local_last_block_id;
    Byte32 first_root_address;

	//门面回调
	IClientCallbackIf* net_proxy;
	IClientTimerIf* timer_proxy;
};

#endif

