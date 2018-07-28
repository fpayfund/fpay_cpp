#include "FPayConfig.h"
#include "FPayTXService.h"

using namespace std;

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
    key.assign(address.u8, sizeof(address.u8));

    if (_balanceCache->get(key, value)) {
        balance = atoll(value.c_str());
        return true;
    }

    return false;
}

bool FPayTXService::handlePayment(const payment_info_t & payment)
{
    uint64_t balance = 0;
    if (!getBalance(payment.pay.from_address) ||
        balance < payment.pay.amount) {
        return false;
    }

    // TODO: calcuate gas

    { // TODO: ensure atom
        string fromKey, toKey;
        fromKey.assign(payment.pay.from_address.u8, sizeof(payment.pay.from_address.u8));
        toKey.assign(payment.pay.to_address.u8, sizeof(payment.pay.to_address.u8));

        int64_t fromVal = 0;
        int64_t toVal = 0;
        _balanceCache->IncrBy(fromKey, -payment.pay.amount, fromVal);
        _balanceCache->IncrBy(toKey, payment.pay.amount, toVal);

        string value;
        PackBuffer pb;
        Pack pk(pb);
        payment.marshall(pk);
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

    for (int i = 0; i < members.size() i++) {
        PackBuffer pb
        pb.append(members[i].c_str(), members[i].size());
        Pack pk(pb);
        payment.unmarshall(pk);
        memPool.push_back(payment);
    }


    return true;
}
