#ifndef FPAY_SERVER_TIMER_H_
#define FPAY_SERVER_TIMER_H_

//上层模块实现该接口(定时器接口）
struct IServerTimerIf {
	//打包时机检测定时器（检测是不是应该开始打包，如果是，则打包区块）
	virtual void onTimerProduceBlockCheck() = 0;
};

#endif

