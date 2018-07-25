#include "common/packet.h"
#include "FPayConfig.h"
#include "FPayBlockService.h"

using namespace std;

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
    _lastBlockId = config->lastBlockId;

    _blockIntervalMS = config->blockIntervalMS;

    _blockCache = RedisCache::create(config->blockCache);
    if (!_blockCache) {
        return false;
    }

    _txService = FPayTXSerivce::getInstance();
    if (!_txService) {
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
    key.assign(id.u8, 32);

    if (!_blockCache->get(key, value)) {
        return false;
    }

    PackBuffer pb
    pb.append(value.c_str(), value.size())
    Pack pk(pb);
    block.unmarshall(pk);

    return true;
}

bool FPayBlockService::getInitBlock(block_info_t & block)
{
    return getBlock(_initBlockId, block);
}

bool FPayBlockService::getLastBlock(block_info_t & block)
{
    return getBlock(_lastBlockId, block);
}

bool FPayBlockService::storeBlock(const block_info_t & block)
{
    if (!_blockCache) {
        return false;
    }

    string key, value;

    PackBuffer pb
    Pack pk(pb);
    block.marshall(pk);
    key.assign(id.u8, 32);
    value.assign(pk.data(), pk.size());

    return _blockCache->set(key, value, uint32_t(-1));
}

bool FPayBlockService::createBlock(block_info_t & block)
{
    block_info_t lastBlock;
    getLastBlock(lastBlock);

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

        for (auto & payment: memPool) {
            block.payments.push_back(payment);
        }

        //genBlockId(block.id);
        block.prev_id = lastBlock.id;
        lastBlock.next_id = block.id;
        storeBlock(block);
        storeBlock(lastBlock);
        return true;
    }

    return false;
}
