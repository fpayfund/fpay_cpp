#ifndef __FPAY_CONFIG_H__
#define __FPAY_CONFIG_H__

#include <string>
#include <vector>
#include <ostream>
#include <fstream>

#include "common/byte.h"

using namespace std;

class FPayConfig
{
public:
    ~FPayConfig();
    bool Load(const char * file);
    static FPayConfig* getInstance();

    int blockInterval;

    Byte32 initBlockId;
    string lastBlockIdCacheKey;

    string blockCache;
    string balanceCache;
    string paymentCache;

    string txPoolCacheKey;

private:
    FPayConfig();

    static FPayConfig* _instance;
};

#endif
