
#ifndef __SOX_SELECTOR_POLL_H_INCLUDE__
#define __SOX_SELECTOR_POLL_H_INCLUDE__

#include "selector_lazy.h"
#include <vector>

namespace sox
{

struct Selector_poll : public Selector_lazy
{
	Selector_poll();
	virtual void select(EV & ready, int timeout);
private:
	std::vector<struct pollfd> m_fds; // XXX C++ -> c
	std::vector<Socket *>      m_conns;
};

} // namespace sox

#endif // __SOX_SELECTOR_POLL_H_INCLUDE__
