#ifndef FPAY_CLIENT_TIMER_IF_H_
#define FPAY_CLIENT_TIMER_IF_H_

//上层模块实现该接口(定时器接口）
struct IClientTimerIf {
  	//根节点切换时机检测定时器
    virtual void onTimerRootSwitchCheck() = 0;
	//区块完整性检查定时器（检测区块是否同步完成，没有同步完成，上层继续发送同步请求）
	virtual void onTimerBlocksFullCheck() = 0;
};

#endif

