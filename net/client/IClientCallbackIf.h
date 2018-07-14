#ifndef FPAY_CLIENT_CALLBACK_IF_H_
#define FPAY_CLIENT_CALLBACK_IF_H_
#include "protocol/fpay_protocol.h"
using namespace ::fpay::protocol;

//上层模块实现该接口
struct IClientCallbackIf {
	
	//子节点注册事件回应
    //virtual int onReceiveChildRegisterRes(const NodeRegisterRes& reg) = 0;
	//父节点连接断开事件
    //virtual void onReceiveParentLeave(Byte32& child_address) = 0;
	//受理支付请求回应
	virtual int onReceivePayRes(const PayRes& res) = 0;
    //矿工节点的同步区块请求回应
	virtual void onReceiveSyncBlock(const block_info_t& block) = 0;
    //查询亲属节点信息回应
	virtual void onReceiveGetRelativesRes(const GetRelativesRes& res) = 0;
};

#endif

