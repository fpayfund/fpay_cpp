#ifndef FPAY_CORE_H_
#define FPAY_CORE_H_
#include <string>
#include <map>

#include "fpay_protocol.h"
#include "common/core/ibase.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "common/core/ilink.h"
#include "common/byte.h"

#include "protocol/fpay_protocol.h"
#include "IServerCallbackIf.h"
#include "IServerTimerIf.h"
#include "IServerSendIf.h"


using namespace ::fpay::protocol; 
using namespace std;

//服务端核心模块
class FPayServerCore:
	public core::PHClass, 
	public core::IFormTarget,
	public core::MultiConnManagerImp
{
	//子节点信息
	typedef struct _child_info
	{
		uint32_t cid;                 //连接id,底层链路ID
		Byte32 address;               //节点地址

		time_t last_ping_time;        //最后心跳时间
		uint64_t pay_count;   //支付确认请求数
		_child_info()
		{
			cid = 0;

			last_ping_time = time(NULL);
			pay_count = 0;
		}
		_child_info(uint32_t c, Byte32 addr):
			cid(c),
			address(addr),
			last_ping_time(time(NULL),
			pay_count(0)
		{
		}
	}child_info_t;


public:
     static FPayServerCore* getInstance()
     {
         if (instance == NULL) {
             instance = new FPayServerCore();
         }
         return instance;
     }

	void init(Byte32 address,
		      Byte32 public_key,
		      Byte32 private_key,
		      IServerCallbackIf* sif,
		      IServerTimerIf* tif);

    //区块广播
     void broadcastBlock(const block_info_t & block);

protected:

	static FPayServerCore* instance;
	//构造，析构
	FPayServerCore();
	~FPayServerCore();

	DECLARE_FORM_MAP
	
	//节点注册到网络
	void onNodeRegister(NodeRegisterReq *reg, core::IConn* c);
	//钱包节点发起支付请求
	void onPay(PayReq* r, core::IConn* c);
	//同步区块请求
	void onSyncBlocks(SyncBlocksReq* r,core::IConn* c);
	//获取亲属节点请求
	void onGetRelatives(GetRelativesReq* r,core::IConn* c);
	//ping 
	void onPing(PingReq* p, core::IConn* c);
	//连接断开事件
	virtual void eraseConnect(IConn *conn);	


	void connHeartbeat(uint32_t cid);
	
	//底层定时器回调
	//定时检测超时子节点
	bool checkChildTimeout();
	//区块打包定时器
	bool checkProduceBlock();

	//子节点信息列表
	map<uint32_t,child_info_t> child_infos;
    //子节点地址到连接的映射
	map<Byte32,/*node address*/ uint32_t/*conn id*/,compByte32> address_2_connid;

	
	//定时器对象
	TimerHandler<FPayServerCore, &FPayServerCore::checkChildTimeout> timer_check_child_timeout;
    TimerHandler<FPayServerCore, &FPayServerCore::checkProduceBlock> timer_check_produce_block;
   
	Byte32 local_address;
	Byte32 local_public_key;
	Byte32 local_private_key;
	//网络事件回调接口
    IServerCallbackIf* net_proxy;
	//定时器时间回调接口
	IServerTimerIf* timer_proxy;
};



#endif

