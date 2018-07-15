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
	typedef struct _up_conn_info
	{
		uint32_t cid;      
        Byte32 address;
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
    FPayClientCore(const Byte32& address,
				const Byte32& public_key,
				const Byte32& private_key,
				IClientCallbackIf* cif,
				IClientTimerIf* tif);
	virtual ~FPayClientCore();



	//初始启动，输入初始启动的节点IP,PORT
	void startSV(const set<node_info_t,compByte32>& init_nodes);


	//对上层暴露的接口
	//转发支付请求
	int dispatchPayReq( const PayReq& pay );
    //同步区块
	int dispatchSyncBlocksReq( const Byte32& from_block_id, uint64_t from_block_idx, uint8_t count );


	//获取本节点树的层级，也即角色
    inline uint8_t getTreeLevel()
	{
		return tree_level;
	}
    //获取初始化进程
	inline uint64_t getInitFlag()
	{
		return init_flag;
	}


protected:
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
	map<uint32_t,up_conn_info_t> up_conn_infos; 
	//备份的上行节点信息
	set<node_info_t,compByte32> backup_node_infos;
	//当前父节点地址
    Byte32 current_parent_address;

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

	//门面回调
	IClientCallbackIf* net_proxy;
	IClientTimerIf* timer_proxy;
};

#endif

