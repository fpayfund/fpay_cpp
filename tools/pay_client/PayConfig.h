#ifndef __FPAY_PAY_CONFIG_H__
#define __FPAY_PAY_CONFIG_H__

#include <string>
#include <vector>
#include <ostream>
#include <fstream>

#include "common/byte.h"

using namespace std;

class PayConfig
{
public:
    ~PayConfig();
    bool Load(const char * file);
    static PayConfig* getInstance();
   
	Byte20 fromAddr; //转出钱包地址
	Byte32 publicKey; //转出钱包公钥
	Byte32 privateKey; //转出钱包公钥
	Byte20 toAddr; //转入钱包地址
	uint64_t amount;//转出金额
private:
    PayConfig();

    static PayConfig* _instance;
};

#endif
