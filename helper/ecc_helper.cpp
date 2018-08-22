#include "ecc_helper.h"
#include "net/common/byte.h"

//生成sha256 哈希值
size_t Hash256(const unsigned char * begin, size_t size, unsigned char to[])
{
	unsigned char hash[HASH256_SIZE];
	if(NULL == begin || size == 0) return 0;
	SHA256(begin, size, (unsigned char *)&hash[0]);
	SHA256(&hash[0], sizeof hash, (unsigned char *)&to[0]);
	return HASH256_SIZE;
}


//生成ripemd160 哈希值
size_t Hash160(const unsigned char * begin, size_t size, unsigned char to[])
{
	unsigned char hash[HASH256_SIZE];
	if(NULL == begin || size == 0) return 0;
	SHA256(begin, size, (unsigned char *)&hash[0]);
	RIPEMD160(&hash[0], sizeof hash, &to[0]);
	return HASH160_SIZE;
}


static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
string Base58Encode(const unsigned char *begin, size_t size)
{

	std::string result = "";
	BN_CTX * ctx = BN_CTX_new();
	BIGNUM bn, dv, rem, bn58, bn0;
	
	BN_init(&bn58); BN_init(&bn0);
	BN_set_word(&bn58, 58);
	BN_zero(&bn0);
	BN_init(&bn); BN_init(&dv); BN_init(&rem);
	BN_bin2bn(begin, size, &bn);
	
	while(BN_cmp(&bn, &bn0) > 0)
	{
		if(!BN_div(&dv, &rem, &bn, &bn58, ctx)) break;
		bn = dv;
		char c = BN_get_word(&rem);
		result += pszBase58[c];
	}
	

	std::string::iterator pbegin = result.begin();
	std::string::iterator pend   = result.end();
	while(pbegin < pend) {
		char c = *pbegin;
		*(pbegin++) = *(--pend);
		*pend = c;
	}
	BN_CTX_free(ctx);
	return result;
}


static const int8_t b58digits[] = {
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
	-1, 0, 1, 2, 3, 4, 5, 6, 7, 8,-1,-1,-1,-1,-1,-1,
	-1, 9,10,11,12,13,14,15, 16,-1,17,18,19,20,21,-1,
	22,23,24,25,26,27,28,29, 30,31,32,-1,-1,-1,-1,-1,
	-1,33,34,35,36,37,38,39, 40,41,42,43,-1,44,45,46,
	47,48,49,50,51,52,53,54, 55,56,57,-1,-1,-1,-1,-1,
};

size_t Base58Decode(const char *begin, size_t size, unsigned char *to)
{
	unsigned char c;
	unsigned char *p = (unsigned char *)begin;
	unsigned char *pend = p + size;
	size_t cb;
	BIGNUM bn, bnchar;
	BIGNUM bn58, bn0;
	BN_CTX *ctx = BN_CTX_new();
	if(NULL == ctx) return 0;
	BN_init(&bn58);
	BN_init(&bn0);
	BN_init(&bn); BN_init(&bnchar);
	BN_set_word(&bn58, 58);
	BN_zero(&bn0);
	while(p < pend)
	{
		c = *p;
		if(c & 0x80) goto label_errexit;
		if(-1 == b58digits[c]) goto label_errexit;
		BN_set_word(&bnchar, b58digits[c]);
		if(!BN_mul(&bn, &bn, &bn58, ctx)) goto label_errexit;
		BN_add(&bn, &bn, &bnchar);
		p++;
	}
	cb = BN_num_bytes(&bn);
	if(NULL == to) return cb;
	BN_bn2bin(&bn, to);
	BN_CTX_free(ctx);
	return cb;
label_errexit:
	if(NULL != ctx) BN_CTX_free(ctx);
	return 0;
}

uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[HASH256_SIZE]);
int ECKey_Check(const unsigned char vch[32]);

//随机产生私钥
uint32_t ECKey_GeneratePrivKey(EC_KEY * pkey, unsigned char vch[32])
{
	//RandAndSeed(); //初始化随机数种子	
	
	do{
		RAND_bytes(vch, 32);//生成256位随机数
	}while(!ECKey_Check(vch));
	if(pkey != NULL) {
		return ECKey_GenKeypair(pkey, vch); //生成密钥对
	}
	return 32; //返回私钥字节数
}


//随机产生一个size字节的大数
bool ECKey_Rand(unsigned char* rand, size_t size)
{
	//RAND_seed
	return RAND_bytes(rand, size) == 1;//生成size字节位随机数
}


void DumpHex(const unsigned char *vch, size_t size)
{
	fprintf(stderr,"-------------size:%Zu-----------\n",size);
	for( uint32_t i =0; i < size; i++ ) {
		if( i % 8 == 0 ) {
			if( i!= 0 ) fprintf(stderr,"\n");
			fprintf(stderr,"%02X ",vch[i]);
		} else {
			fprintf(stderr,"%02X ",vch[i]);
		}
	}
	fprintf(stderr,"\n");

}


int ECKey_Check(const unsigned char vch[32])
{
	static const unsigned char order[32] = {
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
		0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
		0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
	};
	BIGNUM bn; //私钥
	BIGNUM bnOrder; //G 在Fp下的序号order
	BIGNUM bn0; // 0
	BN_init(&bn);
	BN_init(&bnOrder);
	BN_init(&bn0);
	BN_zero(&bn0);
	BN_bin2bn(vch, 32, &bn); //将二进制序列转换为大整数
	BN_bin2bn(order, 32, &bnOrder);
	//0 < [私钥] < order
	if(BN_is_zero(&bn)) return 0; //私钥不能为0
	if(BN_cmp(&bn, &bnOrder) > 0) return 0; //私钥必须小于order
	return 1;
}


//初始化EC_KEY,告知openssl将使用secp256k1椭圆曲线
EC_KEY * ECKey_new()
{
	return EC_KEY_new_by_curve_name(NID_secp256k1);
}

void ECKey_free(EC_KEY* pkey)
{
	EC_KEY_free(pkey);
}

uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[HASH256_SIZE])
{
	const uint32_t key_size = HASH256_SIZE;
	const EC_GROUP * group;
	BIGNUM bn;
	BIGNUM * privkey = &bn;
	BN_CTX * ctx = NULL;
	EC_POINT * pubkey = NULL;
	if(NULL == pkey) return 0;
	BN_init(&bn);
	group = EC_KEY_get0_group(pkey); //group结构体中存储了G值和运算规则
	pubkey = EC_POINT_new(group); //给pubkey分配内存，pubkey为椭圆曲线上的 一个点
	ctx = BN_CTX_new();
	if(NULL == pubkey || NULL == ctx) goto label_errexit;
	if(BN_bin2bn(vch, key_size, &bn)) //将私钥（二进制形式）转换为一个大整形
	{
		if(EC_POINT_mul(group, pubkey, privkey, NULL, NULL, ctx)) // pubkey =privkey*G
		{
			//将密钥对存储在EC_KEY中，便于导出所需格式
			EC_KEY_set_private_key(pkey, privkey);
			EC_KEY_set_public_key(pkey, pubkey);
		}
		BN_clear_free(&bn);
	}
	EC_POINT_free(pubkey);
	BN_CTX_free(ctx);
	return key_size;
label_errexit:
	if(NULL!=pubkey) EC_POINT_free(pubkey);
	if(NULL != ctx) BN_CTX_free(ctx);
	return 0;
}



uint32_t ECKey_GetPubkey(EC_KEY * pkey, unsigned char * pubkey, int fCompressed)
{
	unsigned char * p_pubkey = pubkey;
	uint32_t cb;
	EC_KEY_set_conv_form(pkey, fCompressed?POINT_CONVERSION_COMPRESSED:POINT_CONVERSION_UNCOMPRESSED);
	cb = i2o_ECPublicKey(pkey, NULL);
	if(0==cb || cb>65) return 0;
	if(NULL == pubkey) return cb;
	cb = i2o_ECPublicKey(pkey, &p_pubkey);
	return cb;
}

//公钥地址转换位二进制钱包地址(20byte)
uint32_t PubkeyToBinAddress(const unsigned char * pubkey, size_t size, unsigned char *to)
{
	return Hash160(pubkey, size, to);
}

//将20位二进制钱包地址转换位base58钱包地址
string BinAddressToBase58(const unsigned char *bin_address,size_t size)
{
	return Base58Encode((unsigned char*)bin_address,size);
}

//公钥转换为Base58的钱包地址
string PubkeyToBase58Address(const unsigned char* pubkey, size_t pubkey_size)
{
    unsigned char bin[HASH160_SIZE];
	uint32_t size = PubkeyToBinAddress(pubkey,pubkey_size,bin);
	return BinAddressToBase58(bin,size);
}

//将Base58钱包地址转换成20字节的二进制整形
uint32_t Base58AddressToBin(const string& address,unsigned char* to)
{
	return Base58Decode((const char*)address.c_str(),address.size(),to);
}


//密钥Base58编码
string KeyToBase58(const unsigned char vch[HASH256_SIZE])
{
	return Base58Encode((unsigned char*)vch, HASH256_SIZE);
}

//密钥base58解码
uint32_t KeyFromBase58(const string& base58,unsigned char* vch)
{
	return Base58Decode((const char*)base58.c_str(),base58.size(),vch);
}

uint32_t SignFromBase58(const string& base58,unsigned char* sign)
{
	return Base58Decode((const char*)base58.c_str(),base58.size(),sign);
}

string SignToBase58(const unsigned char* sign, size_t size)
{
	return Base58Encode((unsigned char*)sign, size);
}


//签名 获取64 Byte的(r s)
int ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char *r, unsigned char *s)
{
	int cb = 0;
	ECDSA_SIG *sig = NULL;
	BN_CTX * ctx = NULL;
	BIGNUM order; // The order of G
	BIGNUM halforder; // get sign/unsign mark
	
	const EC_GROUP * group = EC_KEY_get0_group(pkey); // secp256k1: G
	if(NULL == group) return -1;
	sig = ECDSA_do_sign((unsigned char *)&hash[0], HASH256_SIZE, pkey);
	if(NULL == sig) return -1;

	//** sig = (r,s) = (r,-s)
	//** s must less than order/2, otherwise, some app may parse '-s' as a large unsigned positive integer
	ctx = BN_CTX_new();
	if(NULL == ctx) goto label_exit;
	//** allocate memory for bignum
	BN_init(&order);
	BN_init(&halforder);
	// get the order of G
	EC_GROUP_get_order(group, &order, ctx); // secp256k1: n
	BN_rshift1(&halforder, &order);
	if(BN_cmp(sig->s, &halforder)>0)
	{	
		// if s > order/2, then output -s. (-s = (order - s))
		BN_sub(sig->s, &order, sig->s);
	}
	
	BN_CTX_free(ctx);


    BN_bn2bin(sig->r,r);
	BN_bn2bin(sig->s,s);
	
label_exit:
	ECDSA_SIG_free(sig);
	return cb;
}


bool ECKey_Verify(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], const unsigned char *sig, size_t size)
{
	if(ECDSA_verify(0, (unsigned char *)&hash[0], HASH256_SIZE, sig, size, pkey) != 1) // -1 = error, 0 = bad sig, 1 = good
	  return false;
	return true;
}



bool ECKey_Verify(EC_KEY *pkey, 
			const unsigned char hash[HASH256_SIZE], const unsigned char* r, const unsigned char* s)
{
	
	ECDSA_SIG *sig = ECDSA_SIG_new();
    if( sig == NULL ) {
		return false;
	}

	BN_bin2bn(r,32,sig->r);
	BN_bin2bn(s,32,sig->s);


	unsigned char* output = NULL;
 
	unsigned char* sign[1] = {0};
	output = *sign;
	size_t cb = ECDSA_size(pkey);
	if(NULL == output)
	{
		output = (unsigned char *)OPENSSL_malloc(cb);
	}
	if(NULL == *sign) *sign = output;

	cb = i2d_ECDSA_SIG(sig, &output); 
	
	bool ret = ECKey_Verify(pkey,hash,*sign,cb);
    //DumpHex(*sign,cb);
    OPENSSL_free(*sign);
		
	ECDSA_SIG_free(sig);
	return ret;
}


//签名
size_t ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char **to)
{
	size_t cb = 0;
	ECDSA_SIG *sig = NULL;
	BN_CTX * ctx = NULL;
	BIGNUM order; // The order of G
	BIGNUM halforder; // get sign/unsign mark
	unsigned char *output = NULL;
	const EC_GROUP * group = EC_KEY_get0_group(pkey); // secp256k1: G
	if(NULL == group) return 0;
	sig = ECDSA_do_sign((unsigned char *)&hash[0], HASH256_SIZE, pkey);
	if(NULL == sig) return 0;
	//** sig = (r,s) = (r,-s)
	//** s must less than order/2, otherwise, some app may parse '-s' as a large unsigned positive integer
	ctx = BN_CTX_new();
	if(NULL == ctx) goto label_exit;
	//** allocate memory for bignum
	BN_init(&order);
	BN_init(&halforder);
	// get the order of G
	EC_GROUP_get_order(group, &order, ctx); // secp256k1: n
	BN_rshift1(&halforder, &order);
	if(BN_cmp(sig->s, &halforder)>0)
	{
		// if s > order/2, then output -s. (-s = (order - s))
		BN_sub(sig->s, &order, sig->s);
	}

	BN_CTX_free(ctx);
	output = *to;
	cb = ECDSA_size(pkey);
	
	if(NULL == output)
	{
		output = (unsigned char *)OPENSSL_malloc(cb);
		if(NULL == output) goto label_exit;
	}
	if(NULL == *to) *to = output;
	//** i2d_ECDSA_SIG DER encode content of ECDSA_SIG object
	//** (note: this function modifies *pp (*pp += length of the DER encoded signature)).
	//** do not pass the address of 'to' directly
	cb = i2d_ECDSA_SIG(sig, &output);
label_exit:
	ECDSA_SIG_free(sig);
	return cb;
}

/*

void sign1(){
	EC_KEY * ecKey =  ECKey_new();
	unsigned char privBin[32];
	ECKey_GeneratePrivKey(ecKey,privBin);
	uint32_t key_size = ECKey_GenKeypair(ecKey, privBin);
    fprintf(stderr,"key_Size:%u\n",key_size);

	unsigned char pubkey[65] = {0};
    uint32_t pubKeySize = ECKey_GetPubkey(ecKey, pubkey, 1);
	fprintf(stderr,"pubkey size:%u,ECDSA_size:%u\n",pubKeySize,ECDSA_size(ecKey));
   

	string address = PubkeyToBase58Address(&pubkey[1], 32);
	fprintf(stderr,"address:%s\n",address.c_str());
    string sPrivKey = BinAddressToBase58(privBin,32);
	string sPubKey = BinAddressToBase58(&pubkey[1],32);

	fprintf(stderr,"priv key:%s\n",sPrivKey.c_str());
	fprintf(stderr,"pub key:%s\n",sPubKey.c_str());


}


void sign()
{

	EC_KEY * ecKey =  ECKey_new();
	unsigned char vch[32] = { 
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
		0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11
	};
	fprintf(stderr,"------------private key ----------------\n");
	DumpHex(vch,32);
	fprintf(stderr,"------------private key ----------------\n");


	unsigned char pubBin[33] = {
		0x03,0x4F,0x35,0x5B,0xDC,0xB7,0xCC,0x0A,
		0xF7,0x28,0xEF,0x3C,0xCE,0xB9,0x61,0x5D,
		0x90,0x68,0x4B,0xB5,0xB2,0xCA,0x5F,0x85,
		0x9A,0xB0,0xF0,0xB7,0x04,0x07,0x58,0x71,
		0xAA
	};


    unsigned char * pp = (unsigned char*)pubBin;
	unsigned char bin_address[20];
	PubkeyToBinAddress(&pubBin[1],32,bin_address);
    fprintf(stderr,"------------------bin address-----------\n");
	DumpHex(bin_address,20);

	string base58Address = PubkeyToBase58Address(&pubBin[1],32);
	fprintf(stderr,"address:%s\n",base58Address.c_str());

	unsigned char expet_address[20] = {0};
    //uint32_t cb = Base58AddressToBin(base58Address.c_str(),expet_address);
	fprintf(stderr,"---------------expct bin address----------\n");
	DumpHex(expet_address,20);

	//导入私钥
	BIGNUM bn;
	BN_init(&bn);
	BIGNUM* privkey = &bn;
	if( BN_bin2bn(vch, 32, &bn)) //将私钥（二进制形式）转换为一个大整形
	{
		EC_KEY_set_private_key(ecKey, privkey);
	}

	unsigned char hash[32];
	string message ="hello world";
    Hash256((unsigned char*)message.c_str(), message.size(), hash);

	unsigned char* sign[1] = {0};
    size_t sign_Size = ECKey_Sign(ecKey, hash, sign);
    fprintf(stderr,"sign size:%Zu\n",sign_Size);
	fprintf(stderr,"------------sign ----------------\n");	
    DumpHex(*sign,sign_Size);
	fprintf(stderr,"------------sign ----------------\n");
	


	ecKey =  ECKey_new();
	ecKey = o2i_ECPublicKey(&ecKey,(const unsigned char**)&pp,33);
	fprintf(stderr,"ecKey=%p\n",ecKey);
	
	
	bool ret = ECKey_Verify(ecKey, hash, *sign, sign_Size);
	fprintf(stderr,"sign validate:%s\n",ret ? "true":"false");




   
}
*/

/*
void genSign(const Byte32& private_key,Byte64& sign)
{

	unsigned char hash[HASH256_SIZE];
	Hash256((const unsigned char*)"fzh",3,hash);


	EC_KEY* ecKey = ECKey_new();
	if( ecKey == NULL ){
		fprintf(stderr,"genSign,ECKey_new faild\n");
		return;
	}

	BIGNUM bn;
	BN_init(&bn);
	BIGNUM* privkey = &bn;
	if( BN_bin2bn(private_key.u8, 32, &bn)) 
	{
		EC_KEY_set_private_key(ecKey, privkey);
	} else {
		fprintf(stderr,"genSign,BN_bin2bn failed\n");
		goto free;
	}


	ECKey_Sign(ecKey, hash, &(sign.u8[0]), &(sign.u8[0])+32);
free:
	ECKey_free(ecKey);
   
}

bool signValidate(const Byte32& public_key,const Byte64& sign)
{
	unsigned char hash[HASH256_SIZE];
	Hash256((const unsigned char*)"fzh",3,hash);


	EC_KEY* ecKey = ECKey_new();
	if( ecKey == NULL ){
		return false;
	}


	unsigned char pubKey[33];
	pubKey[0] = 0x03; 
	for( uint32_t i = 0; i < 32; i++ ) {
		pubKey[i+1] = public_key.u8[i];
	}
	unsigned char* pp = (unsigned char*)pubKey;
	o2i_ECPublicKey(&ecKey,(const unsigned char**)&pp,33);


	bool ret = ECKey_Verify(ecKey,hash, (const unsigned char*)&(sign.u8[0]), (const unsigned char*)&(sign.u8[0])
				+32);
	ECKey_free(ecKey);

	return ret;
}
*/

/*
int main(int argc,char* argv[])
{
	//sign();
    string privKeyStr = "E7hDgTfbnNZq823gGKTAWukZ1iRApjEdqrZwTjsyegUt";	
    string pubKeyStr = "3C3mG74ZY8t4cAz6oi3KUXxEpFTumGXXaSLMXqBzdmPC";
	Byte32 local_public_key;
	KeyFromBase58(pubKeyStr,local_public_key.u8);
	Byte32 local_private_key;
	KeyFromBase58(privKeyStr,local_private_key.u8);

	Byte64 sign;
	genSign(local_private_key,sign);
	fprintf(stderr,"sign:\n");
	DumpHex(sign.u8,64);

	unsigned char* sign2[1] = {0};
    unsigned char* output = *sign2;
	EC_KEY * pkey = ECKey_new();
	size_t cb = ECDSA_size(pkey);
	
	if(NULL == output)
	{
		fprintf(stderr,"malloc for output\n");
		output = (unsigned char *)OPENSSL_malloc(cb);
		//if(NULL == output) goto label_exit;
	}
	if(NULL == *sign2) *sign2 = output;


	ECDSA_SIG *sig = ECDSA_SIG_new();
    if( sig == NULL ) {
		return false;
	}
	BN_bin2bn(&sign.u8[0],32,sig->r);
	BN_bin2bn(&sign.u8[0]+ 32,32,sig->s);
	
	cb = i2d_ECDSA_SIG(sig, &output); 
	fprintf(stderr,"---------------sign3------------------------\n");
	DumpHex(*sign2,cb);
	DumpHex(output,cb);
	fprintf(stderr,"---------------sign3------------------------\n");


	unsigned char* test = (unsigned char *)OPENSSL_malloc(80);
    OPENSSL_free(test);

	bool sign_validate_ret = signValidate(local_public_key,sign);
	fprintf(stderr,"sign validate ret:%s\n",sign_validate_ret ? "true" : "false");
}
*/
