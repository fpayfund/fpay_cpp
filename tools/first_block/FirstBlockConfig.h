#ifndef __FPAY_FIRST_BLOCK_CONFIG_H__
#define __FPAY_FIRST_BLOCK_CONFIG_H__

#include <string>
#include <vector>
#include <ostream>
#include <fstream>

#include "common/byte.h"

using namespace std;

class FirstBlockConfig
{
public:
    ~FirstBlockConfig();
    bool Load(const char * file);
    static FirstBlockConfig* getInstance();


    Byte32 id;  //创世区块ID
	Byte20 rootAddr; //创世区块根节点地址（上帝钱包地址）
	Byte32 publicKey; //创世区块根节点公钥（上帝钱包公钥）
	uint32_t timestamp; //创世时间戳
	Byte64 sign; //签名

	Byte32 payId; //支付ID
	Byte20 toAddr; //（上帝钱包地址）同上
	uint64_t amount; //转入上帝钱包初始金额，100000000000

    Byte64 paySign;   //支付签名

	string blockCache; //区块缓存服务得配置地址
private:
    FirstBlockConfig();

    static FirstBlockConfig* _instance;
};

#endif
