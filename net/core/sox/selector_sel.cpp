
#include "selector_sel.h"
#include <errno.h>
namespace sox
{
size_t Selector_sel::check_maxconn(size_t max)
{
	if (max == 0 || FD_SETSIZE < max)
	{
#ifndef WIN32
		log(Warn, "reduce maxconn=", max, " -> FD_SETSIZE=", FD_SETSIZE);
#endif
		return FD_SETSIZE;
	}
	return max;
}

void Selector_sel::select(EV & ready, int timeout)
{
	SOCKET max = -1;
	fd_set rfds, wfds;
	FD_ZERO(&rfds);
	FD_ZERO(&wfds);

	SelMap::iterator it;
	for (it = m_selects.begin(); it != m_selects.end(); ++it)
	{
		if (!it->second) continue;
		assert(it->first->socket().isValid());

		SOCKET s = it->first->socket().getsocket();
		// XXX check FD_SETSIZE
		if (it->second & SEL_READ)  FD_SET(s, &rfds);
		if (it->second & SEL_WRITE) FD_SET(s, &wfds);
		if (max < s) max = s;
	}

	struct timeval tvo;
	if (timeout >= 0)
	{
		tvo.tv_sec = timeout / 1000;
		tvo.tv_usec = (timeout % 1000) * 1000;
	}
	int nready = ::select(int(max) + 1, &rfds, &wfds, NULL, (timeout < 0 ? NULL: &tvo));
	if (nready < 0)
	{
		if (errno == EINTR)
			return;
		throw exception_errno(socket_error::getLastError(), "select");
	}

	for (it = m_selects.begin();
		nready > 0 && it != m_selects.end(); ++it)
	{
		SOCKET s = it->first->socket().getsocket();

		bool b=false;
		if (FD_ISSET(s, &rfds))
		{
			b=true;
			ready.push_back(Event(it->first, SEL_READ));
		}
		if (FD_ISSET(s, &wfds))
		{
			b=true;
			ready.push_back(Event(it->first, SEL_WRITE));
		}
		if (b)
			-- nready;
	}
}

} // namespace sox


