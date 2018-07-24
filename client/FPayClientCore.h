#ifndef __FPAY_CLIENT_CORE_H_
#define __FPAY_CLIENT_CORE_H_
#include "common/core/iserver.h"
#include "common/core/ibase.h"
#include "common/protocol/prouter.h"
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

	static FPayClientCore* getInstance()
	{
		if (instance == NULL) {
			instance = new FPayClientCore();
		}
		return instance;
	}

	void init(const Byte20& address,
				const Byte32& public_key,
				const Byte32& private_key);	

	//初始启动，输入初始启动的节点IP,PORT
	void start(const vector< pair<string,uint16_t> >& init_nodes);

	//对上层暴露的接口
	//转发支付请求
	int dispatchPay( const PayReq& pay );


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
    //收到心跳回应
	void onPingRes(PingRes* res, IConn* c);
    //连接抛上来的事件
    virtual void eraseConnect(core::IConn *conn); //连接断开事件
    virtual void onError(int ev, const char *msg, core::IConn *conn); //连接错误


	//连接器
    core::IConn *connectNode(const string& ip, uint16_t port);
    //选举新的父节点
    void voteParentNode();
	void send(uint32_t cid, uint32_t uri, sox::Marshallable& marshal);
	void registerIn(const string& ip, uint16_t port);
	void syncBlocks(uint32_t cid, 
				const Byte32& from_block_id, 
				uint64_t from_block_idx,
				uint8_t count);
	uint32_t findConnByAddress(const Byte20& address);


	//底层定时器回调函数
    bool linkCheck(); 
    bool ping();
	//根节点切换定时器
	bool checkRootSwitch();
	//区块完整性检查定时器
	bool checkBlocksFull();
	//路由优化检查定时器
	bool checkBestRoute();


protected:
				
	FPayClientCore();
	~FPayClientCore();
    static FPayClientCore* instance;	
	//client模块初始化进度标志
	uint64_t init_flag;
	//树的层级，0为根节点
    uint8_t tree_level;
	//已经连上的上行节点列表信息
	map<uint32_t,up_conn_info_t> up_conn_infos; 
	//备份的上行节点信息
	set<node_info_t,nodeInfoCmp> backup_node_infos;
	//当前父节点地址
    Byte32 current_parent_address;

	//定时器对象
    TimerHandler<FPayClientCore, &FPayClientCore::linkCheck> timer_link_check; 
    TimerHandler<FPayClientCore, &FPayClientCore::ping> timer_ping;
	TimerHandler<FPayClientCore, &FPayClientCore::checkRootSwitch> timer_check_root_switch;
	TimerHandler<FPayClientCore, &FPayClientCore::checkBlocksFull> timer_check_blocks_full;
	TimerHandler<FPayClientCore, &FPayClientCore::checkBestRoute> timer_check_best_route;
	
    //初始信息
	Byte20 local_address;  //本矿工节点地址
	Byte32 local_public_key; //本矿工节点的公钥
	Byte32 local_private_key; //本矿工节点的私钥
   
};

#endif

