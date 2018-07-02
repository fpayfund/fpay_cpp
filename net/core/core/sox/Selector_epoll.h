#ifndef SELECTOR_EPOLL_H_
#define SELECTOR_EPOLL_H_

#include "selector.h"

#include "util.h"

#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#include <set>

namespace sox {
class Selector_epoll : public sox::Selector {
	int epfd;
	
	typedef std::set<Socket *> sockSet_t;
	sockSet_t sockets;
public:
	Selector_epoll();
	virtual ~Selector_epoll();

	virtual void select(Socket * s, int remove, int add);
	virtual void mainloop();

	void remove(Socket * s);
	
	virtual std::ostream& trace(std::ostream &os) const{return os;}
};
}
#endif /*SELECTOR_EPOLL_H_*/
