
#include <poll.h>
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

#include "selector_poll.h"

namespace sox
{

static void poll_timer_click(int)
{
	if(sox::env::selector()) 
		sox::env::selector()->increase_elapsed(Countdown::TIME_CLICK);
}

Selector_poll::Selector_poll()
{
	struct itimerval timer;

	timer.it_interval.tv_sec = Countdown::TIME_CLICK / 1000;
	timer.it_interval.tv_usec = (Countdown::TIME_CLICK % 1000) * 1000;

	timer.it_value = timer.it_interval;

	if (signal(SIGALRM, poll_timer_click) == SIG_ERR)
		throw exception_errno("signal");
	if (-1 == setitimer(ITIMER_REAL, &timer, NULL))
		throw exception_errno("setitimer");
	
}

void Selector_poll::select(EV & ready, int timeout)
{
	if (m_fds.size() < m_selects.size())
	{
		m_fds.resize(m_selects.size());
		m_conns.resize(m_selects.size());
	}
	int nfds = 0;
	for (SelMap::iterator it = m_selects.begin(); it != m_selects.end(); ++it)
	{
		if (!it->second) continue;

		m_conns[nfds] = it->first;
		m_fds[nfds].fd = it->first->socket().getsocket();
		m_fds[nfds].events = 0;
		if (it->second & SEL_READ) m_fds[nfds].events |= POLLIN;
		if (it->second & SEL_WRITE) m_fds[nfds].events |= POLLOUT;
		nfds ++;
	}

	int nready = ::poll(&m_fds[0], nfds, timeout);
	if (nready < 0)
	{
		if (errno == EINTR)
			return;
		throw exception_errno("poll");
	}

	for (int i=0; i<nfds && nready > 0; ++i)
	{
		assert(m_conns[i]->socket().getsocket() == m_fds[i].fd);
		/*
		if (m_fds[i].revents & (POLLERR | POLLNVAL))
		{
			// XXX
			--nready;
			sox::log(Error, "POLLERR |POLLNVAL ", m_fds[i].revents);
			m_conns[i]->destroy();
			continue;
		}
		*/
		if (m_fds[i].revents & (POLLIN|POLLERR|POLLNVAL|POLLHUP))
			ready.push_back(Event(m_conns[i], SEL_READ));

		if (m_fds[i].revents & POLLOUT)
			ready.push_back(Event(m_conns[i], SEL_WRITE));

		if (m_fds[i].revents) // XXX all handled ??
			--nready;
	}
}

} // namespace sox
