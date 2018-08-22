#ifndef __FPAY_BLOCK_SERVICE_H__
#define __FPAY_BLOCK_SERVICE_H__

#include <time.h>

#include "protocol/fpay_protocol.h"
#include "cache/Cache.h"

#include "FPayTXService.h"

class FPayBlockService
{
    public:
        ~FPayBlockService();
        bool init();
        bool getBlock(const Byte32 & id, block_info_t & block);
        bool getInitBlock(block_info_t & block);
        bool getLastBlock(block_info_t & block);
        bool storeBlock(const block_info_t & block);
        bool createBlock(block_info_t & block,const Byte32& private_key);
        bool removeBlock(const block_info_t & block);

		bool storeLastBlockId(const Byte32& id);

        uint32_t timestamp()
        {
            //return clock();
			return time(NULL);
        }

        static FPayBlockService* getInstance()
        {
            if (!_instance) {
                _instance = new FPayBlockService();
                if (!_instance->init()) {
                    delete _instance;
                    _instance = NULL;
                    return NULL;
                }
            }

            return _instance;
        }

    private:
        FPayBlockService();

        bool genBlockId(Byte32 & id);

        static FPayBlockService* _instance;

        uint32_t _blockIntervalMS;

        Byte32 _initBlockId;
        string _lastBlockIdCacheKey;

        Cache* _blockCache;
        FPayTXService* _txService;
};

#endif
