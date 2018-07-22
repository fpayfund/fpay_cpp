#ifndef __NU_SELECTOR_EPOLL_H
#define __NU_SELECTOR_EPOLL_H

#include "selector_lazy.h"
#include <sys/epoll.h>


#define EPOLL_QUEUE_SIZE 0x01111
namespace sox
{

class NuSelectorEpoll : public Selector_lazy
{
public:
	NuSelectorEpoll(int maxFdCount = EPOLL_QUEUE_SIZE);
	~NuSelectorEpoll(void);
	virtual void select(Socket *, int, int);
	virtual void remove(Socket *);
	virtual void mainloop();

	virtual void select(EV & ready, int timeout){}

protected:
	void epollNotify(Socket* s, int ev);

protected:
	int m_epoll_fd;
	int m_maxfds;
};

}

#endif
