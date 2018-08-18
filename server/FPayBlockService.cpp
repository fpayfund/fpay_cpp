#include "common/packet.h"
#include "ecc_helper.h"
#include "FPayConfig.h"
#include "FPayBlockService.h"

using namespace std;
using namespace sox;

FPayBlockService* FPayBlockService::_instance = NULL;
static FPayConfig* config = FPayConfig::getInstance();

FPayBlockService::FPayBlockService()
        : _blockIntervalMS(500)
        , _blockCache(NULL)
        , _txService(NULL)
{
}

FPayBlockService::~FPayBlockService()
{
    if (_blockCache) {
        delete _blockCache;
        _blockCache = NULL;
    }

    _txService = NULL;
}

bool FPayBlockService::init()
{
    _initBlockId = config->initBlockId;
    _lastBlockCacheId = config->lastBlockCacheId;

    _blockIntervalMS = config->blockInterval;

	fprintf(stderr, "FPayBLOCKservice::init\n");
    _blockCache = Cache::create(config->blockCache);
    if (!_blockCache) {
		fprintf(stderr,"Cache create failed\n");
        return false;
    }

    _txService = FPayTXService::getInstance();
    if (!_txService) {
		fprintf(stderr,"TX SERVICE FAILED\n");
        return false;
    }

    return true;
}

bool FPayBlockService::getBlock(const Byte32 & id, block_info_t & block)
{
    if (!_blockCache) {
        return false;
    }

    string key, value;
    key.assign((char*)id.u8, sizeof(id.u8));

	fprintf(stderr,"block id:\n");
	DumpHex(id.u8,32);

	fprintf(stderr,"FPayBlockService::getBlock\n");
    if (!_blockCache->get(key, value)) {
        return false;
    }
  	fprintf(stderr,"FPayBlockService::getBlock,return,value size:%Zu\n",value.size()); 
    //PackBuffer pb;
    //pb.append(value.c_str(), value.size());
	try {
      Unpack up(value.c_str(),value.size());
       block.unmarshal(up);
	}catch(...){
		fprintf(stderr,"some exception\n");
	}
    fprintf(stderr,"FPayBlockService::getBlock,unpack\n");
    return true;
}

bool FPayBlockService::getInitBlock(block_info_t & block)
{
    return getBlock(_initBlockId, block);
}

bool FPayBlockService::getLastBlock(block_info_t & block)
{
	fprintf(stderr,"FPayBlockService::getLastBlock\n");
    return getBlock(_lastBlockCacheId, block);
}

bool FPayBlockService::storeBlock(const block_info_t & block)
{
    if (!_blockCache) {
        return false;
    }

    string key, value;

    PackBuffer pb;
    Pack pk(pb);
    block.marshal(pk);
    key.assign((char *)block.id.u8, sizeof(block.id.u8));
    value.assign(pk.data(), pk.size());

    return _blockCache->set(key, value, uint32_t(-1));
}

bool FPayBlockService::storeLastBlock(const block_info_t & block)
{
    if (!_blockCache) {
        return false;
    }

    string key, value;

    PackBuffer pb;
    Pack pk(pb);
    block.marshal(pk);
    key.assign((char*)_lastBlockCacheId.u8, sizeof(_lastBlockCacheId.u8));
    value.assign(pk.data(), pk.size());

    return _blockCache->set(key, value, uint32_t(-1));
}

bool FPayBlockService::removeBlock(const block_info_t & block)
{
    if (!_blockCache) {
        return false;
    }

    string key;
    key.assign((char*)block.id.u8, sizeof(block.id.u8));

    return _blockCache->remove(key);
}

bool FPayBlockService::genBlockId(Byte32 & id)
{
	return ECKey_Rand(id.u8,32);
}

bool FPayBlockService::createBlock(block_info_t & block,const Byte32& private_key)
{
    block_info_t lastBlock;
    if (!getLastBlock(lastBlock)) {
        return false;
    }

    uint64_t ts = timestamp();
    if (lastBlock.timestamp - ts > _blockIntervalMS) {
        return false;
    }

    block.timestamp = ts;

    vector<payment_info_t> memPool;
    if (_txService->getMemoryPool(memPool)) {
        if (memPool.empty()) {
            return false;
        }

        for (uint32_t i = 0; i < memPool.size(); i++) {
            block.payments.push_back(memPool[i]);
        }

        genBlockId(block.id);
        block.pre_id = lastBlock.id;
        lastBlock.next_id = block.id;

		block.genSign(private_key);
        if (!storeBlock(block)) {
            return false;
        }
        if (!storeBlock(lastBlock)) {
            removeBlock(block);
            return false;
        }
        
        storeLastBlock(lastBlock);
        return true;
    }

    return false;
}
