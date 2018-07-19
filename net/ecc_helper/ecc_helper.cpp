#include "ecc_helper.h"
#include <openssl/md5.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include "core/common/byte.h"

#define HASH256_SIZE (32)
#define HASH160_SIZE (20)

size_t Hash256(const unsigned char * begin, size_t size, unsigned char to[])
{
	unsigned char hash[HASH256_SIZE];
	if(NULL == begin || size == 0) return 0;
	SHA256(begin, size, (unsigned char *)&hash[0]);
	SHA256(&hash[0], sizeof hash, (unsigned char *)&to[0]);
	return HASH256_SIZE;
}

size_t Hash160(const unsigned char * begin, size_t size, unsigned char to[])
{
	unsigned char hash[HASH256_SIZE];
	if(NULL == begin || size == 0) return 0;
	SHA256(begin, size, (unsigned char *)&hash[0]);
	RIPEMD160(&hash[0], sizeof hash, &to[0]);
	return HASH160_SIZE;
}


static const char* pszBase58 = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
size_t Base58Encode(const unsigned char *begin, size_t size, char *to)
{
	size_t cb = 0;
	BN_CTX * ctx = NULL;
	unsigned char c;
	unsigned char *pzero = (unsigned char *)begin;
	unsigned char *pend = (unsigned char *)(begin + size);
	char *p = to;
	// bool fSign = 0;
	BIGNUM bn, dv, rem, bn58, bn0;
	if((NULL == begin) || (size == 0)) return 0; // invalid parameter
	cb = size * 138 /100+1; // the size of output will less than (138/100 * sizeof(src))
	//** output buffer should be allocated enough memory
	if(NULL == to) return cb;
	BN_init(&bn58); BN_init(&bn0);
	BN_set_word(&bn58, 58);
	BN_zero(&bn0);
	BN_init(&bn); BN_init(&dv); BN_init(&rem);
	BN_bin2bn(begin, size, &bn);
	ctx = BN_CTX_new();
	if(NULL==ctx) return 0;

	while(BN_cmp(&bn, &bn0) > 0)
	{
		if(!BN_div(&dv, &rem, &bn, &bn58, ctx)) break;
		bn = dv;
		c = BN_get_word(&rem);
		*(p++) = pszBase58[c];
	}
	// Ìí¼ÓÇ°µ¼µÄ0
	while(*(pzero++)==0)
	{
		*(p++) = pszBase58[0];
		if(pzero > pend) break;
	}
	*p = '';
	cb = p - to;
	ReverseBytes((unsigned char *)to, cb); // output as a big endian integer
	BN_CTX_free(ctx);
	return cb;
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


BOOL ECKey_Check(const unsigned char vch[32]);
uint32_t ECKey_GeneratePrivKey(EC_KEY * pkey, unsigned char vch[32])
{
	RandAndSeed(); //¿¿¿¿¿¿¿¿¿¿¿¿¿¿bitcoin
	do
	{
		RAND_bytes(vch, 32); //¿¿openssl¿¿¿¿256¿¿¿¿¿
	}while(!ECKey_Check(vch));//¿¿¿¿¿¿¿¿¿¿
	if(pkey != NULL) return ECKey_GenKeypair(pkey, vch); //¿¿¿¿¿
	return 32; //¿¿¿¿¿¿
}


BOOL ECKey_Check(const unsigned char vch[32])
{
	static const unsigned char order[32] = {
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
		0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
		0xBA,0xAE,0xDC,0xE6,0xAF,0x48,0xA0,0x3B,
		0xBF,0xD2,0x5E,0x8C,0xD0,0x36,0x41,0x40
	};
	BIGNUM bn; //¿¿
	BIGNUM bnOrder; //
	BIGNUM bn0; // 0
	BN_init(&bn);
	BN_init(&bnOrder);
	BN_init(&bn0);
	BN_zero(&bn0);
	BN_bin2bn(vch, 32, &bn); //¿¿¿¿¿¿¿¿¿¿¿¿¿¿
	BN_bin2bn(order, 32, &bnOrder);
	//0 < [¿¿] < order
	if(BN_is_zero(&bn)) return 0; //¿¿¿¿¿0
	if(BN_cmp(&bn, &bnOrder) > 0) return 0; //¿¿¿¿¿¿order
	return 1;
}


//¿¿¿EC_KEY,¿openssl¿secp256k1¿¿¿¿
EC_KEY * ECKey_new()
{
	return EC_KEY_new_by_curve_name(NID_secp256k1);
}


uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[HASH256_SIZE])
{
	//** pkey = ECKey_new();
	const uint32_t key_size = HASH256_SIZE;
	const EC_GROUP * group;
	BIGNUM bn;
	BIGNUM * privkey = &bn;
	BN_CTX * ctx = NULL;
	EC_POINT * pubkey = NULL;
	if(NULL == pkey) return 0;
	BN_init(&bn); //BIGNUM¿¿¿
	group = EC_KEY_get0_group(pkey); //group¿¿¿¿¿¿¿G¿¿¿¿¿¿
	pubkey = EC_POINT_new(group); //¿pubkey¿¿¿¿¿pubkey¿¿¿¿¿¿¿¿
	ctx = BN_CTX_new();
	if(NULL == pubkey || NULL == ctx) goto label_errexit;
	if(BN_bin2bn(vch, key_size, &bn)) //¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿
	{
		if(EC_POINT_mul(group, pubkey, privkey, NULL, NULL, ctx)) // pubkey =privkey*G
		{
			//¿¿¿¿¿EC_KEY¿¿¿¿¿¿¿¿¿¿¿¿¿¿
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



uint32_t ECKey_GetPubkey(EC_KEY * pkey, unsigned char * pubkey, BOOL fCompressed)
{
	unsigned char * p_pubkey = pubkey;
	uint32_t cb;
	EC_KEY_set_conv_form(pkey, fCompressed?POINT_CONVERSION_COMPRESSED:POINT_CONVERSION_UNCOMPRESSED);
	cb = i2o_ECPublicKey(pkey, NULL);
	if(0==cb || cb>65) return 0;
	if(NULL == pubkey) return cb;
	cb = i2o_ECPublicKey(pkey, &p_pubkey);
	return cb
}


//**************************************************
//** Base58check±àÂëÉú³É±ÈÌØ±ÒµØÖ·
uint32_t PubkeyToAddr(const unsigned char * pubkey, size_t size, char *to)
{
	struct
	{
		unsigned char version;
		unsigned char vch[20];
		unsigned char checksum[4];
	}ext_pubkey;
	unsigned char hash[32]={0};
	printf("ext_pubkey size: %d\n", sizeof(ext_pubkey));
	ext_pubkey.version = 0x00;
	Hash160(pubkey, size, &ext_pubkey.vch[0]);
	Hash256(&ext_pubkey.version, 1+20, hash);
	memcpy(ext_pubkey.checksum, hash, 4);
	return Base58Encode((unsigned char *)&ext_pubkey.version, sizeof(ext_pubkey), to);
}



uint32_t PrivkeyToWIF(const unsigned char vch[HASH256_SIZE], char *to, BOOL fCompressed)
{
	struct
	{
		unsigned char version;
		unsigned char vch[HASH256_SIZE];
		unsigned char checksum[5];
	}ext_privkey;
	unsigned char hash[HASH256_SIZE];
	uint32_t offset = fCompressed?1:0;
	ext_privkey.version = 0x80;
	memcpy(ext_privkey.vch, vch, 32);
	if(offset) ext_privkey.checksum[0] = 0x01;
	Hash256(&ext_privkey.version, 1 + HASH256_SIZE +offset, hash);
	memcpy(&ext_privkey.checksum[offset], hash, 4 );
	return Base58Encode(&ext_privkey.version, 1 + HASH256_SIZE + offset + 4, to);
}



size_t ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char **to)
{
	//** if (*to) == NULL, the caller should use OPENSSL_free(*to) to free the memory
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


BOOL ECKey_Verify(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], const unsigned char *sig, size_t size)
{
	if(ECDSA_verify(0, (unsigned char *)&hash[0], HASH256_SIZE, sig, size, pkey) != 1) // -1 = error, 0 = bad sig, 1 = good
	  return FALSE;
	return TRUE;
}
