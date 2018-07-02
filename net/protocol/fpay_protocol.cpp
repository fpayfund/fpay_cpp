#include "fpay_protocol.h"


//节点注册 数据签名验证
bool NodeRegisterReq::signValidate()
{

	return true;
}

//支付请求的数据签名验证
bool PayReq::signValidate()
{

	return true;
}
