#ifndef __FPAY_SERVER_CORE_H_
#define __FPAY_SERVER_CORE_H_
#include "fpay_protocol.h"
#include "common/core/ibase.h"
#include "core/corelib/MultiConnManagerImp.h"
#include "common/core/ilink.h"
#include "helper/TimerHandler.h"
#include "protocol/fpay_protocol.h"
#include <string>
#include <map>


using namespace ::fpay::protocol; 
using namespace std;

//单例模式，上层模块可以直接调用接口
//服务端核心模块
class FPayServerCore:
	public core::PHClass, 
	public core::IFormTarget,
	public core::MultiConnManagerImp
{


	//连接信息
	typedef struct _conn_info
	{
		uint32_t cid;
		time_t connected_timestamp;
		string ip;
	}conn_info_t;

	//子节点连接信息
	typedef struct _child_info
	{
		uint32_t cid;                 //连接id,底层链路ID
		Byte20 address;               //节点地址
		string ip;                    //对端地址
		time_t connected_timestamp;   //连接建立时间戳
		time_t register_timestamp;    //连接注册时间戳
		time_t last_ping_timestamp;   //最后心跳时间
		uint64_t pay_count;           //支付请求数
		_child_info()
		{
			cid = 0;
			time_t now = time(NULL);
			connected_timestamp = now;
			register_timestamp = now;
			last_ping_timestamp = now;
			pay_count = 0;
		}
		_child_info(uint32_t c, const Byte20& addr):
			cid(c),
			address(addr)
		{
			time_t now = time(NULL);
			connected_timestamp = register_timestamp = last_ping_timestamp = now;
			pay_count = 0;	
		}
	}child_info_t;
 

	//父节点 广播区块的连接，由父节点主动发起的连接
	typedef struct _parent_info
	{
		uint32_t cid;
		Byte32 address;
		string ip;
		time_t connected_timestamp;
		time_t first_broadcast_timestamp;
		time_t last_broadcast_timestamp;
		uint64_t broadcast_count;
		_parent_info()
		{
			cid = 0;
			time_t now = time(NULL);
			connected_timestamp = first_broadcast_timestamp = last_broadcast_timestamp = now;
			broadcast_count = 0;
		}

	}parent_info_t;

public:
     static FPayServerCore* getInstance()
     {
         if (_instance == NULL) {
             _instance = new FPayServerCore();
         }
         return _instance;
     }

	void init(const Byte20& address,
		      const Byte32& public_key,
		      const Byte32& private_key);

	 //区块广播
    void broadcastBlock(const block_info_t & block);


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
    //收到父节点发来的区块
	void onBlockBroadcast(BlockBroadcast* broadcast, core::IConn* conn);

	//连接断开事件
	virtual void eraseConnect(core::IConn *conn);	
	//连接建立事件
	virtual void onConnCreate(core::IConn *conn);
	
	
protected:

	static FPayServerCore* _instance;
	//构造，析构
	FPayServerCore();
	~FPayServerCore();

	bool connHeartbeat(uint32_t cid);
    void response(uint32_t cid, uint32_t uri, sox::Marshallable& marshal);
	//底层定时器回调
	//定时检测超时子节点
	bool checkChildTimeout();
	//区块打包定时器
	bool checkCreateBlock();


	//连接信息表
	map<uint32_t,conn_info_t> _connInfos;
	//父节点连接信息,只能存在一个父节点
	parent_info_t _parentInfo;
	//子节点信息列表
	map<uint32_t,child_info_t> _childInfos;

    	
	//定时器对象
	TimerHandler<FPayServerCore, &FPayServerCore::checkChildTimeout> _timerCheckChildTimeout;
    TimerHandler<FPayServerCore, &FPayServerCore::checkCreateBlock> _timerCheckCreateBlock;
   
	Byte20 _localAddress;
	Byte32 _localPublicKey;
	Byte32 _localPrivateKey;

};



#endif

