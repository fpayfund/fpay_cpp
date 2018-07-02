
#include "selector_kq.h"
#include "snox.h"

namespace sox
{

/*
static void kq_sighandler(int sig)
{
	// Do nothing here
}
*/

Selector_kq::Selector_kq()
{
	m_nchange = 0;
	m_max_active = 0;
	m_active = 0;
	m_max_change = 0;
	m_event.resize(INIT_EVENT_SIZE);

	m_kqueue = kqueue();
	if (m_kqueue == -1)
		throw exception_errno("kqueue");

	struct itimerval timer;

	timer.it_interval.tv_sec = Countdown::TIME_CLICK / 1000;
	timer.it_interval.tv_usec = (Countdown::TIME_CLICK % 1000) * 1000;

	timer.it_value = timer.it_interval;

	if (signal(SIGALRM, SIG_IGN /* kq_sighandler */) == SIG_ERR)
		throw exception_errno("signal");

	setup(NULL, SIGALRM, EVFILT_SIGNAL, EV_ADD);

	if (-1 == setitimer(ITIMER_REAL, &timer, NULL))
		throw exception_errno("setitimer");
}

Selector_kq::~Selector_kq()
{
	close(m_kqueue);
}

void Selector_kq::select(Socket * s, int remove, int add)
{
	//remove = remove & ~add; // do not remove the add
	if (SEL_READ & remove)	setup(s, s->socket().getsocket(), EVFILT_READ, EV_DELETE);
	if (SEL_WRITE & remove) setup(s, s->socket().getsocket(), EVFILT_WRITE, EV_DELETE);

	// setup add
	if (SEL_READ & add)  setup(s, s->socket().getsocket(), EVFILT_READ, EV_ADD);
	if (SEL_WRITE & add) setup(s, s->socket().getsocket(), EVFILT_WRITE, EV_ADD);
}

void Selector_kq::mainloop()
{
	while (isRunning())
	{
		// at least can receive all error
		if (m_event.size() < m_nchange) m_event.resize(m_nchange);
		m_active = kevent(m_kqueue, change(), m_nchange, event(), m_event.size(), NULL);
		m_nchange = 0;

		if (m_active < 0)
		{
			if (errno == EINTR) continue;
			throw exception_errno("kevent");
		}

		if (m_active > m_max_active) m_max_active = m_active;

		for (int i = 0; i<m_active; ++i)
		{
			if (m_event[i].flags & EV_ERROR)
			{
				check_error(m_event[i]);
				continue;
			}

			switch (m_event[i].filter)
			{
			case EVFILT_READ:
				notify_event((Socket *)(m_event[i].udata), SEL_READ);
				break;

			case EVFILT_WRITE:
				notify_event((Socket *)(m_event[i].udata), SEL_WRITE);
				break;

			case EVFILT_SIGNAL:
				assert(m_event[i].ident == SIGALRM);
				sox::env::now = time(NULL);
				timout_run(Countdown::TIME_CLICK);
				interrupt();
				break;

			default:
				throw exception_errno(0, "unknown event");
			}
		}
		clearRemoved();
	}
}

std::ostream & Selector_kq::trace(std::ostream & os) const
{
	return os << "active=" << m_active << "/" << m_max_active
		<< " max_change=" << m_max_change;
}


/*
 * EV_ERROR
 * check_error(int e)

 * Error messages that can happen, when a delete fails.
 *   EBADF happens when the file discriptor has been closed,
 *   ENOENT when the file discriptor was closed and then reopened.
 * An error is also indicated when a callback deletes
 * an event we are still processing.  In that case
 * the data field is set to ENOENT.
 *

 * delay-setup. selectting will save in m_change
 * before setup into kernel . Socket maybe 
 * 1 delete
 * 2 close
 * ignore these routing.

 *
 * SAMPLE:
 * Socket * s;	// fd == 1
 * select(s,..);
 * s->close(); // will call remove
 * s->attach(accept()); // newfd == 1;
 * select(s,..); // resetup

 */

void Selector_kq::check_error(struct kevent & e)
{
	exception_errno err(e.data, "kevent EV_ERROR");
	if (e.data == EBADF || e.data == ENOENT)
		sox::log(Info, err.what_str(), e.ident);
	else
		throw err;
}

} // namespace sox
