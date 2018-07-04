#include "fpay_protocol.h"


//确认数据签名验证
bool confirmation_info_t::signValidate()
{
	return true;
}


//支付数据签名验证
bool pay_t::signValidate()
{

	return true;
}

//支付体签名验证
bool payment_info_t::signValidate()
{
	if( pay.signValidate() ) {
		for( uint32_t i = 0; i <confirmations.size(); i++ ) {
			if( !confirmations[i].signValidate() ) return false;
		}
		return true;
	}

	return false;
}

//区块签名验证
bool block_info_t::signValidate()
{
	for( uint32_t i = 0; i < payments.size(); i++ ) {
		if( ! payments[i].signValidate() ) return false;
	}
	//to do 验证本签名数据
	return true;
}

//节点注册 数据签名验证
bool NodeRegisterReq::signValidate()
{
	return true;
}

//节点注册返回 数据签名验证
bool NodeRegisterRes::signValidate() {
	return true;
}


//支付请求的数据签名验证
bool PayReq::signValidate()
{
	if( pay.signValidate() ) {
		for( uint32_t i = 0; i < confirm_link.size(); ++i ) {
			if( ! confirm_link[i].signValidate() ) return false;
		}
		return true;
	}
	return false;
}

