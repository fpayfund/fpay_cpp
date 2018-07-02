
#include "countdown.h"
#include "snox.h"
//#define CT_TRACE 0
namespace sox
{

#ifdef CT_TRACE

int tick_count = 0;

#define DUMP_TIMEOUT()   trace(std::cout, true)<<"-- timeout --"<<tick_count<<"\n";
#define DUMP_FALLDOWN()  trace(std::cout)<<"-- fall --"<<tick_count<<"\n";
#define INC_TICK_COUNT() tick_count++;
#define DUMP_SELECT()    trace(std::cout, true)<<"-- select --"<<tick_count<<"\n";

#else

#define DUMP_TIMEOUT() 
#define DUMP_FALLDOWN()
#define INC_TICK_COUNT()
#define DUMP_SELECT()

#endif

void Countdown::select_timeout(Handler * s, int timeout)
{
	iterator & pos = s->m_iter_timo;
	if (timeout < 0) // remove
	{
		if (isRegister(pos))
		{
			m_falls[pos->fallidx].erase(pos);
			pos = null_iter();
		}
		return;
	}

	size_t newidx = find_fall_idx(timeout);
	timeout += m_falls[newidx].m_elasped;

	if (!isRegister(pos)) // not register, first time
		pos = m_falls[newidx].insert(Timeout((int)newidx, s, timeout));
	else
	{
		if (size_t(pos->fallidx) != newidx)
		{
			m_falls[newidx].splice(m_falls[pos->fallidx].m_list, pos);
			pos->fallidx = (int)newidx;
		}
		pos->count = timeout;
	}
	DUMP_SELECT();
}

void Countdown::click_elapse(timeout_result & result, int elasped)
{
	INC_TICK_COUNT();
	//assert(elasped > 0);

	// >>>>>> 这种写法必须有bottom 2, 否则注释掉这个，下面扫描fall改成从idx=0开始
	_list & list = m_falls[0].m_list;
	if (!list.empty())
	{
		for (_list_iter i = list.begin(); i != list.end(); ++i)
			i->handler->m_iter_timo = null_iter(); // reset
		result.splice(result.end(), list);
		DUMP_TIMEOUT();
	}
	// <<<<<<

	for (size_t idx = 1; idx <= m_falls_last_idx; ++idx)
	{
		m_falls[idx].m_elasped += elasped;
		if (m_falls[idx].m_elasped < m_falls[idx].m_fall)
			continue;

		_list & list = m_falls[idx].m_list;
		for (_list_iter i = list.begin(); i != list.end(); /* nothing */)
		{
			i->count -= m_falls[idx].m_elasped;
			if (i->count <= 0) // timeout
			{
				i->handler->m_iter_timo = null_iter(); // reset
				result.splice(result.end(), list, i++);
				DUMP_TIMEOUT();
				continue;
			}

			size_t newidx = find_fall_idx(i->count, idx);
			if (newidx == idx) // bottom always ok
				++i; // fall no change
			else
			{
				i->count += m_falls[newidx].m_elasped;
				i->fallidx = (int)newidx;
				m_falls[newidx].splice(list, i++);
				DUMP_FALLDOWN();
			}
		}
		m_falls[idx].m_elasped = 0;
	}		
}

Countdown::Countdown()
{
	m_falls.push_back(Fall(0));             // bottom 0
	m_falls.push_back(Fall(TIME_CLICK * 2)); // bottom 2

	// *2 方式建立 fall
	m_falls.push_back(Fall(TIME_CLICK * 5));
	m_falls.push_back(Fall(TIME_CLICK * 11));
	m_falls.push_back(Fall(TIME_CLICK * 23));
	m_falls.push_back(Fall(TIME_CLICK * 47));
	m_falls.push_back(Fall(TIME_CLICK * 97));
	m_falls.push_back(Fall(TIME_CLICK * 197));
	m_falls.push_back(Fall(TIME_CLICK * 397));

	m_falls_last_idx = m_falls.size() - 1;
}

std::ostream & Countdown::trace(std::ostream & os, bool dumpall) const
{
	for (Falls::const_iterator i = m_falls.begin(); i != m_falls.end(); ++i)
	{
		os << "list=" << i->m_fall << ":" << i->m_elasped << "\t";
		if (dumpall)
			for (_list::const_iterator ii = i->m_list.begin(); ii != i->m_list.end(); ++ii)
				os << ii->count << "\t";
		else
			os <<(unsigned int) i->m_list.size();
		os << "\n";
	}
	return os;
}

} // namespace sox
