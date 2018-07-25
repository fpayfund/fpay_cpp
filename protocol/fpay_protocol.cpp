#include "fpay_protocol.h"
#include "ecc_helper.h"
#include "common/packet.h"

using namespace sox;


static bool signValidate(Pack& pk,const Byte32& public_key,const Byte64& sign)
{
	unsigned char hash[HASH256_SIZE];
	Hash256((const unsigned char*)pk.data(),pk.size(),hash);

	//初始化椭圆曲线
	EC_KEY* ecKey = ECKey_new();
	if( ecKey == NULL ){
		return false;
	}

	//导入公钥
	unsigned char pubKey[33];
	pubKey[0] = 0x03; //压缩格式
	for( uint32_t i = 0; i < 32; i++ ) {
		pubKey[i+1] = public_key.u8[i];
	}
	unsigned char* pp = (unsigned char*)pubKey;
	o2i_ECPublicKey(&ecKey,(const unsigned char**)&pp,33);

	//签名验证
	bool ret = ECKey_Verify(ecKey,hash, (const unsigned char*)&(sign.u8[0]), (const unsigned char*)&(sign.u8[0])+32);
	ECKey_free(ecKey);

	return ret;
}


static void genSign(const Pack& pk,const Byte32& private_key,Byte64& sign)
{

	//计算hash256值
	unsigned char hash[HASH256_SIZE];
	Hash256((const unsigned char*)pk.data(),pk.size(),hash);

	//初始化椭圆曲线
	EC_KEY* ecKey = ECKey_new();
	if( ecKey == NULL ){
		return;
	}
	//导入私钥
	BIGNUM bn;
	BN_init(&bn);
	BIGNUM* privkey = &bn;
	if( BN_bin2bn(private_key.u8, 32, &bn)) //将私钥（二进制形式）转换为一个大整形
	{
		EC_KEY_set_private_key(ecKey, privkey);
	} else {
		goto free;
	}

	//生成签名sign的前32个字节存储r 后32个字节存储s
	ECKey_Sign(ecKey, hash, &(sign.u8[0]), &(sign.u8[0])+32);
free:
	ECKey_free(ecKey);
}

namespace fpay { namespace protocol {
	//确认数据签名验证
	bool confirmation_info_t::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << current_address << timestamp << balance << payment_id << next_address;

		return ::signValidate(pk,public_key,sign);	
	}


	void  confirmation_info_t::genSign(const Byte32& private_key)
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << current_address << timestamp << balance << payment_id << next_address;

		::genSign(pk,private_key,sign);
	}

	//支付数据签名验证
	bool pay_t::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << id << from_address << to_address;
		pk << amount << balance << balance_payment_id << accept_address;

		return ::signValidate(pk,public_key,sign);	
	}

	void pay_t::genSign(const Byte32& private_key)
	{

		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << id << from_address << to_address;
		pk << amount << balance << balance_payment_id << accept_address;

		::genSign(pk,private_key,sign);
	}

	//支付体签名验证
	bool payment_info_t::signValidate()
	{
		if( pay.signValidate() ) {
			for( uint32_t i = 0; i < confirmations.size(); i++ ) {
				if( !confirmations[i].signValidate() ) return false;
			}
			return true;
		}

		return false;
	}

	//区块签名验证
	bool block_info_t::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << idx << id << pre_id << root_address  << timestamp;
		
		if( ::signValidate(pk,public_key,sign) ) {	
			for( uint32_t i = 0; i < payments.size(); i++ ) {
				if( ! payments[i].signValidate() ) return false;
			}
		}	
		return true;
	}

	void block_info_t::genSign(const Byte32& private_key)
	{

		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << idx << id << pre_id << root_address << timestamp;
		
		::genSign(pk,private_key,sign);
	}

	//节点注册 数据签名验证
	bool NodeRegisterReq::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << ip << port;
		
		return ::signValidate(pk,public_key,sign);	
	}


	void NodeRegisterReq::genSign(const Byte32& private_key)
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << ip << port;
		::genSign(pk,private_key,sign);
	}

	//节点注册返回 数据签名验证
	bool NodeRegisterRes::signValidate() 
	{

		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << tree_level << last_block_idx << last_block_id;
		return ::signValidate(pk,public_key,sign);	

	}

	void NodeRegisterRes::genSign(const Byte32& private_key)
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << tree_level << last_block_idx << last_block_id;
		::genSign(pk,private_key,sign);

	}


	//支付请求的数据签名验证
	bool PayReq::signValidate()
	{
		return payment.signValidate();
	}

	bool PayRes::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << resp_code << id;
		return ::signValidate(pk,public_key,sign);	
	}

	void PayRes::genSign(const Byte32& private_key)
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << resp_code << id;
		::genSign(pk,private_key,sign);

	}

	bool BlockBroadcast::signValidate()
	{
		return block.signValidate();
	}

	bool SyncBlocksReq::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << from_block_id << from_block_idx << block_num;
		return ::signValidate(pk,public_key,sign);	
	}

	void SyncBlocksReq::genSign(const Byte32& private_key)
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		PackBuffer pb;
		Pack pk(pb);
		pk << from_block_id << from_block_idx << block_num;
		::genSign(pk,private_key,sign);

	}


	bool SyncBlocksRes::signValidate()
	{
		//序列化所需签名验证的字段，这里的验证序列化和生成签名的序列化的字段和顺序要完全一致
		//否则签名验证失败
		for( uint32_t i = 0; i < blocks.size(); i++ )
		{
			if( blocks[i].signValidate() == false) return false;
		}

		return true;
	}

}
}
