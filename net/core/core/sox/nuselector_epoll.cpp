#include "nuselector_epoll.h"
#include <time.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>

namespace sox
{

static void epoll_timer_click(int) {
	if (sox::env::selector()) {
			sox::env::now = time(NULL);
			char *st = ctime(&sox::env::now);
			sox::env::strTime = std::string(st, strlen(st) - 1);
			sox::env::selector()->increase_elapsed(Countdown::TIME_CLICK);
	}
}
NuSelectorEpoll::NuSelectorEpoll(int maxFdCount)
{
		struct itimerval timer;

		timer.it_interval.tv_sec = Countdown::TIME_CLICK / 1000;
		timer.it_interval.tv_usec = (Countdown::TIME_CLICK % 1000) * 1000;

		timer.it_value = timer.it_interval;

		if (signal(SIGALRM, epoll_timer_click) == SIG_ERR)
				throw exception_errno("signal");
		if (-1 == setitimer(ITIMER_REAL, &timer, NULL))
				throw exception_errno("setitimer");

		m_maxfds = maxFdCount;			
		m_epoll_fd = -1;
		if(-1 == (m_epoll_fd = epoll_create(m_maxfds)))
		{
				sox::log(Debug, "epoll_create failed");
		}
}
NuSelectorEpoll::~NuSelectorEpoll(void)
{
		close(m_epoll_fd);

}
void NuSelectorEpoll::select(Socket* s, int remove, int add)
{
		sox::log(Debug, "NuEpoll:select");
		sox::log(Debug, add);

		std::pair<SelMap::iterator, bool> p = m_selects.insert(SelMap::value_type(s, 0));

		int bAdd = p.second;

		struct epoll_event ep;
		memset(&ep, 0, sizeof(ep));

		ep.events = p.first->second;

		if (add & SEL_READ)  ep.events |= EPOLLIN;
		if (add & SEL_WRITE) ep.events |= EPOLLOUT;

		if(remove & SEL_READ) ep.events &= ~EPOLLIN;
		if(remove & SEL_WRITE) ep.events &= ~EPOLLOUT;

//		ep.events |= EPOLLERR | EPOLLHUP /* | EPOLLET */;

		sox::log(Debug, ep.events);


		ep.data.ptr = s;
		SOCKET fd = s->socket().getsocket();

		if (0 != epoll_ctl(m_epoll_fd, bAdd ? EPOLL_CTL_ADD : EPOLL_CTL_MOD, fd, &ep)) {
				sox::log(Debug, "epoll_ctl select failed");
				return;
		}			
		p.first->second=ep.events;

		sox::log(Debug, "NuEpoll:select success");
}
void NuSelectorEpoll::remove(Socket* sock)
{
		sox::log(Debug, "NuEpoll:remove");
	 	epoll_event ep = { 0, { 0 } };

		SOCKET fd = sock->socket().getsocket();

		if (0 != epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, fd, &ep)) {
				sox::log(Debug, "epoll_ctl remove failed");
		}
		m_selects.erase(sock);
		sox::log(Debug, "NuEpoll:remove success");

}
void NuSelectorEpoll::mainloop()
{
		epoll_event events[128];
		while(isRunning()) {
				//sox::log(Debug, "epoll_ctl begin wait .........");
				int nfds = epoll_wait(m_epoll_fd, events, 128, 100);
				//sox::log(Debug, "epoll_ctl finish wait .........");

				if(nfds)
					sox::log(Debug, "some event occur: ", nfds);

				for(int n = 0; n < nfds; ++n) {
						struct epoll_event*  ev = &events[n];
						epollNotify((Socket*)ev->data.ptr, ev->events); // please ensure the socket pointer is valid now?
				}

				//sox::log(Debug, "check timeout run .........");
				check_timout_run(); // check timeout
				clearRemoved();
		}
}

void NuSelectorEpoll::epollNotify(Socket* s, int ev)
{
		sox:log(Debug, "events notify is:", ev);
		if(ev & EPOLLIN) notify_event(s, SEL_READ);
		if(ev & EPOLLOUT) notify_event(s, SEL_WRITE);
}

}
