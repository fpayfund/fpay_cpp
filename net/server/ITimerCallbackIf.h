#ifndef FPAY_TIMER_CALLBACK_IF_H_
#define FPAY_TIMER_CALLBACK_IF_H_

//上层模块实现该接口(定时器接口）
struct ITimerCallbackIf {
  	//根节点切换时机检测定时器
    virtual void onTimerRootSwitchCheck() = 0;
	//区块完整性检查定时器（检测区块是否同步完成，没有同步完成，上层继续发送同步请求）
	virtual void onTimerBlocksFullCheck() = 0;
	//打包时机检测定时器（检测是不是应该开始打包，如果是，则打包区块）
	virtual void onTimerProduceBlockCheck() = 0;

	//定时检测是否有更好的路由（更好的父节点）
	virtual void onTimerBestRouteCheck() = 0;
};

#endif

