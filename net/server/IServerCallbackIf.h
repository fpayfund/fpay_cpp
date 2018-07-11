#ifndef FPAY_SERVER_CALLBACK_IF_H_
#define FPAY_SERVER_CALLBACK_IF_H_
#include "protocol/fpay_protocol.h"
using namespace ::fpay::protocol;

//上层模块实现该接口
struct IServerCallbackIf {
	
	//子节点注册事件
    virtual int onReceiveChildRegister(const NodeRegisterReq& reg, NodeRegisterRes& res);
	//子节点连接断开事件
    virtual void onReceiveChildLeave(Byte32& child_address);
	//受理支付请求
	virtual int onReceivePay(const PayReq& pay, PayRes& res) = 0;
    //矿工节点的同步区块请求
	virtual int onReceiveSyncBlocks(const SyncBlocksReq& sync,SyncBlocksRes& res) = 0;
    //查询亲属节点信息
	virtual int onReceiveGetRelatives(const GetRelativesReq& get, GetRelativesRes& res) = 0;
};

#endif

