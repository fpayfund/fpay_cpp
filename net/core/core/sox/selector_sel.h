
#ifndef __SOX_SELECTOR_SEL_H_INCLUDE__
#define __SOX_SELECTOR_SEL_H_INCLUDE__

#include "selector_lazy.h"

namespace sox
{

struct Selector_sel : public Selector_lazy
{
	Selector_sel() { m_maxconns = check_maxconn(m_maxconns); }
	virtual void set_max_connections(size_t max) { m_maxconns = check_maxconn(max); }
	virtual void select(EV & ready, int timeout);
protected:
	size_t check_maxconn(size_t max);
};

} // namespace sox

#endif // __SOX_SELECTOR_SEL_H_INCLUDE__
