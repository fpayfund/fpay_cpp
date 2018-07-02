#ifndef FPAY_SERVER_SEND_IF_H_
#define FPAY_SERVER_SEND_IF_H_

//区块广播接口
struct IBlockBroadcastIf
{
	//广播新的区块
	virtual void broadcastBlock(const BlockBroadcast& broadcast) = 0;
};

struct IServerResponseIf
{
	//异步调用注册回应
	virtual void sendRegisterRes(const Byte32& address,const NodeRegisterRes& res) = 0;
	//异步调用支付回应
	virtual void sendPayRes(const Byte32& address, const PayRes& res ) = 0;

    //异步调用确认回应
	virtual void sendComfirmRes(const Byte32& address,const ConfirmRes& res) = 0;

	//异步回应同步区块请求
	virtual void sendSyncBlocksRes(const Byte32& address, const SyncBlocksRes& res) = 0;
}
#endif

