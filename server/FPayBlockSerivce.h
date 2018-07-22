#ifndef __FPAY_BLOCK_SERVICE_H__
#define __FPAY_BLOCK_SERVICE_H__

#include "protocol/fpay_protocol.h"
#include "cache/Cache.h"

class FPayBlockSerivce
{
    public:
        ~FPayBlockService();
        bool init();
        bool getBlock(const Byte32 id, block_info_t & block);
        bool getInitBlock(block_info_t & block);
        bool getLastBlock(block_info_t & block);
        bool storeBlock(const block_info_t & block);
        bool createBlock(block_info_t & block);

        static FPayBlockSerivce* getInstance()
        {
            if (!_instance) {
                _instance = new FPayBlockService();
                if (!_instance->init()) {
                    delete _instance;
                    return NULL;
                }
            }

            return _instance;
        }

    private:
        FPayBlockService()
        {}

	    static FPayBlockService* _instance;

        Byte32 _initBlockId;
        Byte32 _lastBlockId;

        uint32_t _blockIntervalMS;

        Cache* _blockCache;
};

#endif
