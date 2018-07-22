
#ifndef __SOX_SELECTOR_KQ_H_INCLUDE__
#define __SOX_SELECTOR_KQ_H_INCLUDE__

#include "snox.h"
#include "util.h"

#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include <set>

namespace sox
{

class Selector_kq : public Selector
{
public:
	Selector_kq();
	virtual ~Selector_kq();
	virtual void select(Socket * s, int remove, int add);
	virtual void mainloop();
	virtual std::ostream & trace(std::ostream & os) const;

protected:
	int m_max_active;
	int m_active;

	size_t m_max_change;
	size_t m_nchange;

	int m_kqueue;

	void check_error(struct kevent & e);

	typedef std::vector<struct kevent> Events;
	Events m_change; // XXX C++ -> c
	Events m_event;  // XXX C++ -> c
	struct kevent * change() { return &m_change[0]; }
	struct kevent * event()  { return &m_event[0]; }

	enum { INC_CHANGE_SIZE = 100, INIT_EVENT_SIZE = 1000 };
	void setup(Socket * s, int fd, short filter, u_short flags)
	{
		if (m_nchange == m_change.size())
			m_change.resize(m_nchange + INC_CHANGE_SIZE);
		EV_SET(&m_change[m_nchange], fd, filter, flags, 0, 0, s);
		if (++m_nchange > m_max_change) m_max_change = m_nchange;
	}
};

} // namespace sox

#endif // __SOX_SELECTOR_KQ_H_INCLUDE__
