#ifndef ASTATLINKHANDLER_H_
#define ASTATLINKHANDLER_H_
#include "common/core/ilink.h"
#include "common/int_types.h"

namespace core{
	class AStatLinkHandler: public ILinkHandler, public IProcStat{
		uint64_t totalProc;

		uint64_t flowStat;

		uint32_t intevelProc;

		uint32_t limitSize;
	public:
		AStatLinkHandler();

		virtual uint32_t getAndRstIntevel();

		virtual uint64_t getTotal();

		virtual uint64_t getFlow();

		virtual void incProc();

		virtual void incProc(uint32_t PktSize);

		virtual void setPackLimit(uint32_t sz);

		inline uint32_t getPackLimit() const{
			return limitSize;
		}
	};
}
#endif

