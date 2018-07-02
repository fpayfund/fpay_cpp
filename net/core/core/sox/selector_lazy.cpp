#include "core/sox/selector_lazy.h"
#include <stdexcept>

namespace sox
{

void Selector_lazy::remove(Socket * s)
{
	m_selects.erase(s);
}

void Selector_lazy::select(Socket * s, int remove, int add)
{
	std::pair<SelMap::iterator, bool> p = m_selects.insert(SelMap::value_type(s, 0));

	// XXX can't check before insert. new or modify are same
	if (	m_maxconns > 0 && // 为 0 时不限制数量
		p.second // 新加入的时候检查数量
		&& m_selects.size() > m_maxconns // 是否超过限制
		)
	{
		m_selects.erase(p.first); // setup failed . clear inner
		throw std::runtime_error("too many connections");
	}

	p.first->second = (p.first->second & ~remove) | add;
}

std::ostream & Selector_lazy::trace(std::ostream & os) const
{
	return os;
}

Selector_lazy::~Selector_lazy()
{
	// XXX DEBUG ONLY
	//while (!m_selects.empty())
	//	m_selects.begin()->first->destroy();
}

void Selector_lazy::mainloop()
{
	EV ready;
	while (isRunning())
	{
		//log(Debug, "select at ", env::now);
		select(ready, 500);
		sox::env::now = time(NULL);
		char *st = ctime(&sox::env::now);
		sox::env::strTime = std::string(st, strlen(st) - 1);

		//for china
		sox::env::haomiao =  (int)(sox::env::now % (24 * 3600) + 8 * 3600);

		
		for (EV::iterator it = ready.begin(); it != ready.end(); ++it)
			notify_event(it->first, it->second);
		ready.clear(); // notify complete
		check_timout_run(); // check timeout
		interrupt();
		clearRemoved(); // remove safe now
	}
}

} // namespace sox
