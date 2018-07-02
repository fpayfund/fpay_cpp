
#ifndef __SOX_XTIMER_H_INCLUDE__
#define __SOX_XTIMER_H_INCLUDE__

#include "snox.h"

namespace sox
{

namespace detail
{

class timer_base : public sox::Handler
{
public:
	timer_base(int interval) : m_interval(interval) { }

	int  getinterval() const       { return m_interval; }
	void setinterval(int interval) { m_interval = interval; }
	void restart()                 { select_timeout(m_interval); }

protected:
	int m_interval;
};

// XXX
//
template < class OpCallback >
class timer_impl : public timer_base
{
public:
	timer_impl(const OpCallback & op, int interval)
		: timer_base(interval), callback(op)
	{
		restart();
	}

protected:
	virtual void handle(int ev)
	{
		try
		{
			callback();
		}
		catch (std::exception & ex)
		{
			log(Error, "XTimer callback exception ", ex.what());
		}
		if (m_interval > 0)
			restart();
	}
	OpCallback callback;
}; 

} // namespace detail

typedef detail::timer_base xtimer;

template < class OpCallback >
inline xtimer * new_xtimer(const OpCallback & op, int interval)
{
	return new detail::timer_impl<OpCallback>(op, interval);
}

} // namespace sox

#endif // __SOX_XTIMER_H_INCLUDE__
