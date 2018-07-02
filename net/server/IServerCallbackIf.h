#ifndef FPAY_SERVER_CALLBACK_IF_H_
#define FPAY_SERVER_CALLBACK_IF_H_

//上层模块实现该接口
struct IServerCallbackIf {
  	//节点注册事件
    virtual void onReceiveRegister(const NodeRegisterReq& reg);
	//受理支付请求
	virtual void onReceivePay(const PayReq& pay) = 0;
    //矿工节点的同步区块请求
	virtual void onReceiveSyncBlocks(const SyncBlocksReq& sync) = 0;
    //查询亲属节点信息
	virtual void onReceiveGetRelatives(const GetRelativesReq& get) = 0;
};

#endif

