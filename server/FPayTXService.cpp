#include "common/packet.h"
#include "FPayConfig.h"
#include "helper/ecc_helper.h"
#include "FPayTXService.h"


using namespace std;
using namespace sox;

FPayTXService* FPayTXService::_instance = NULL;
static FPayConfig* config = FPayConfig::getInstance();

bool FPayTXService::init()
{
    _balanceCache = Cache::create(config->balanceCache);
    if (!_balanceCache) {
        return false;
    }

    _paymentCache = Cache::create(config->paymentCache);
    if (!_paymentCache) {
        return false;
    }

    // TODO: load block and initiate balance

    return true;
}

bool FPayTXService::getBalance(const Byte20 & address, uint64_t & balance)
{
    if (!_balanceCache)
        return false;

    string key, value;
    key.assign((char*)address.u8, sizeof(address.u8));

    if (_balanceCache->get(key, value)) {
        balance = atoll(value.c_str());
        return true;
    }

    return false;
}

bool FPayTXService::updateBalanceByBlock(const block_info_t & block)
{
    if (!_balanceCache)
        return false;

    map<string, int64_t> balanceMap;

    for (uint32_t i = 0; i < block.payments.size(); i++) {
        string from, to;
        payment_info_t payment = block.payments[i];
        from.assign((char*)payment.pay.from_address.u8, sizeof(payment.pay.from_address.u8));
        to.assign((char*)payment.pay.to_address.u8, sizeof(payment.pay.to_address.u8));
        balanceMap[from] -= payment.pay.amount;
        balanceMap[to] += payment.pay.amount;

		fprintf(stderr,"FPayTXService::updateBalanceByBlock,from address:%s,to address:%s,amount:%lu\n",
					BinAddressToBase58(payment.pay.from_address.u8,32).c_str(),BinAddressToBase58(payment.pay.to_address.u8,32).c_str(),payment.pay.amount); 
	}

    for (map<string, int64_t>::iterator iter = balanceMap.begin(); iter != balanceMap.end(); iter++) {
        int64_t balance = 0;
        _balanceCache->incrBy(iter->first, iter->second, balance);
    }

    return false;
}

bool FPayTXService::handlePayment(const payment_info_t & payment)
{
    uint64_t balance = 0;
    if (!getBalance(payment.pay.from_address, balance) ||
        balance < payment.pay.amount) {
		fprintf(stderr,"FPayTXService::handlePayment,from address:%s,to address:%s,balance:%lu,need pay amount:%lu\n",
					BinAddressToBase58(payment.pay.from_address.u8,32).c_str(),BinAddressToBase58(payment.pay.to_address.u8,32).c_str(),balance,payment.pay.amount);
        return false;
    }

    // TODO: calcuate gas

    { // TODO: ensure atom
        string fromKey, toKey;
        fromKey.assign((char*)payment.pay.from_address.u8, sizeof(payment.pay.from_address.u8));
        toKey.assign((char*)payment.pay.to_address.u8, sizeof(payment.pay.to_address.u8));

        int64_t fromVal = 0;
        int64_t toVal = 0;
        _balanceCache->incrBy(fromKey, -payment.pay.amount, fromVal);
        _balanceCache->incrBy(toKey, payment.pay.amount, toVal);

        string value;
        PackBuffer pb;
        Pack pk(pb);
        payment.marshal(pk);
        value.assign(pk.data(), pk.size());

        _paymentCache->enqueue(config->txPoolCacheKey, value);
    }
    
    return true;
}

bool FPayTXService::getMemoryPool(vector<payment_info_t> & memPool)
{
    if (!_paymentCache) {
        return false;
    }

    vector<string> members;
    if (!_paymentCache->dequeueAll(config->txPoolCacheKey, members)) {
        return false;
    }

    payment_info_t payment;

    for (uint32_t i = 0; i < members.size(); i++) {
        Unpack pk(members[i].data(), members[i].size());
        payment.unmarshal(pk);
        memPool.push_back(payment);
    }


    return true;
}
