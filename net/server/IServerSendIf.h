#ifndef FPAY_SERVER_SEND_IF_H_
#define FPAY_SERVER_SEND_IF_H_

//区块广播接口
struct IBlockBroadcastIf
{
	//广播新的区块
	virtual void broadcastBlock(const BlockBroadcast& broadcast) = 0;
};

#endif

