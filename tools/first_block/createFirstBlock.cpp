#include <string>
#include "FirstBlockConfig.h"
#include "Cache.h"
#include "ecc_helper.h"
#include "fpay_protocol.h"
using namespace fpay::protocol;
using namespace sox;
using namespace std;
int main(int argc, char* argv[])
{
	if( argc != 2 ) {
		fprintf(stderr,"usage:private_key_base58\n");
		return 1;
	}
	string private_key_base58 = argv[1];
	Byte32 private_key;
	KeyFromBase58(private_key_base58,private_key.u8);
	if( FirstBlockConfig::getInstance()->Load("first_block.xml") == false ) {

		fprintf(stderr, "load first_block.xml config failed\n");
		return 1;
	}
	FirstBlockConfig* config = FirstBlockConfig::getInstance();

	Cache* blockCache = Cache::create(config->blockCache);
	if (!blockCache) {
		fprintf(stderr,"Cache create failed\n");
		return 1;
	}

	payment_info_t payment;
    payment.pay.id = config->payId;
	payment.pay.public_key = config->publicKey;
	payment.pay.timestamp = config->timestamp;
	payment.pay.to_address = config->rootAddr;
	payment.pay.amount = config->amount;
	payment.pay.balance = 0;

	if( config->paySign.isEmpty() ) {
		payment.pay.genSign(private_key);
		string sSign = SignToBase58(payment.pay.sign.u8,64);
		fprintf(stderr,"pay sign:\n");
		fprintf(stderr,"%s\n",sSign.c_str());

	} else {
	    payment.pay.sign = config->paySign;
    }

	block_info_t block;
    block.idx = 0;
	block.id = config->id;
	block.root_address = config->rootAddr;
	block.public_key = config->publicKey;
	block.timestamp = config->timestamp;
	block.payments.push_back(payment);
	if( config->sign.isEmpty() ) {
		block.genSign(private_key);
		string sSign = SignToBase58(block.sign.u8,64);
		fprintf(stderr,"block sign:\n");
		fprintf(stderr,"%s\n",sSign.c_str());
 
	} else {
	    block.sign = config->sign;
   
	}

	string key, value;
	PackBuffer pb;
	Pack pk(pb);
	block.marshal(pk);
	key.assign((char*)block.id.u8, sizeof(block.id.u8));
	value.assign(pk.data(), pk.size());
	blockCache->set(key, value, uint32_t(-1));

	return 0;
}
