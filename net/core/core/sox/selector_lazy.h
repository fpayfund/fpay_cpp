
#ifndef __SOX_SELECTOR_LAZY_H_INCLUDE___
#define __SOX_SELECTOR_LAZY_H_INCLUDE___

#include <map>

#include "snox.h"

namespace sox
{

class Selector_lazy : public Selector
{
public:
	Selector_lazy() { m_maxconns = 0; }
	virtual ~Selector_lazy();
	virtual void select(Socket *, int, int);
	virtual void remove(Socket *);
	virtual void mainloop();
	virtual std::ostream & trace(std::ostream & os) const;

protected:
	typedef std::map<Socket *, int> SelMap;
	typedef std::pair<Socket *, int> Event;
	typedef std::vector<Event> EV;
	virtual void select(EV & ready, int timeout) = 0;

	SelMap m_selects;
	size_t m_maxconns;
};

} // namespace sox

#endif // __SOX_SELECTOR_LAZY_H_INCLUDE___
