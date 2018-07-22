#ifndef __FPAY_TX_SERVICE_H__
#define __FPAY_TX_SERVICE_H__

#include "protocol/fpay_protocol.h"
#include "cache/Cache.h"

class FPayTXSerivce
{
    public:
        ~FPayTXService();

        bool init();

        bool handlePayment(const payment_info_t & payment);

        static FPayTXSerivce* getInstance()
        {
            if (!_instance) {
                _instance = new FPayTXService();
                if (!_instance->init()) {
                    delete _instance;
                    return NULL;
                }
            }

            return _instance;
        }

    private:
        FPayTXService()
            : _balanceCache(NULL)
            , _paymentCache(NULL)
        {}

	    static FPayTXService* _instance;

        Cache* _balanceCache;
        Cache* _paymentCache;
};

#endif
