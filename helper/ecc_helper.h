#ifndef __FPAY_ECC_HELPER_H_
#define __FPAY_ECC_HELPER_H_
#include <openssl/md5.h>
#include <string.h>
#include <string>
#include <openssl/bn.h>
#include <openssl/sha.h>
#include <openssl/ec.h>
#include <openssl/ripemd.h>
#include <openssl/obj_mac.h>
#include <stdint.h>
#include <openssl/rand.h>
#include <openssl/ecdsa.h>

//hash256 位
#define HASH256_SIZE (32)
////hash160 位
#define HASH160_SIZE (20)

using namespace std;
//生成hash256 值
size_t Hash256(const unsigned char * begin, size_t size, unsigned char to[]);

//生成ripemd160 哈希值
size_t Hash160(const unsigned char * begin, size_t size, unsigned char to[]);

//Base58 编码
string Base58Encode(const unsigned char *begin, size_t size);

//base58 解码
size_t Base58Decode(const char *begin, size_t size, unsigned char *to);

//二进制 打印十六进制文本
void DumpHex(const unsigned char *vch, size_t size);

//随机产生一个size字节大小的随机大数
bool ECKey_Rand(unsigned char* rand, size_t size);

//初始化EC_KEY,告知openssl将使用secp256k1椭圆曲线
EC_KEY * ECKey_new();
void ECKey_free(EC_KEY* pkey);

//公钥地址转换位二进制钱包地址(20byte)
uint32_t PubkeyToBinAddress(const unsigned char * pubkey, size_t size, unsigned char *to);

//将20位二进制钱包地址转换位base58钱包地址
string BinAddressToBase58(const unsigned char *bin_address,size_t size);

//公钥转换为Base58的钱包地址
string PubkeyToBase58Address(const unsigned char* pubkey, size_t pubkey_size);

//将Base58钱包地址转换成20字节的二进制整形
uint32_t Base58AddressToBin(const string& address,unsigned char* to);

//密钥Base58编码
string keyToBase58(const unsigned char vch[HASH256_SIZE]);

//密钥base58解码
uint32_t KeyFromBase58(const string& base58,unsigned char* vch);

uint32_t SignFromBase58(const string& base58,unsigned char* sign);

//签名 获取64 Byte的(r s)
int ECKey_Sign(EC_KEY *pkey, const unsigned char hash[HASH256_SIZE], unsigned char *r, unsigned char *s);

//签名验证 sign的 r 和 s
bool ECKey_Verify(EC_KEY *pkey,const unsigned char hash[HASH256_SIZE], const unsigned char* r, const unsigned char* s);

//生产随机私钥
uint32_t ECKey_GeneratePrivKey(EC_KEY * pkey, unsigned char vch[32]);

//根据私钥获取公钥
uint32_t ECKey_GetPubkey(EC_KEY * pkey, unsigned char * pubkey, int fCompressed);

uint32_t ECKey_GenKeypair(EC_KEY * pkey, unsigned char vch[HASH256_SIZE]);

#endif

