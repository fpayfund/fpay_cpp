#include "Selector_epoll.h"
#include "snox.h"
#include <sys/epoll.h>

#define EPOLL_SIZE 100
static int pipe_err = 0;
using namespace sox;

static void epoll_timer_click(int) {
	if (sox::env::selector()) {
		sox::env::selector()->increase_elapsed(Countdown::TIME_CLICK);
	}
}

static void epoll_pipe_handler(int){
	pipe_err++;
}

Selector_epoll::Selector_epoll() :
	epfd(-1) {

	if(signal(SIGPIPE, epoll_pipe_handler) == SIG_ERR){
		throw exception_errno("signal");
	}

	epfd = epoll_create(65535);
	if (-1 == epfd) {
		throw exception_errno("epoll");
	}

}

Selector_epoll::~Selector_epoll() {
	if (-1 != epfd)
		close(epfd);
}

inline void setupEpoll(int efd, int op, int sfd, epoll_event &ev) {
	int ret = epoll_ctl(efd, op, sfd, &ev);
	if (ret != 0) {
		switch (errno) {
		case EBADF:
			log(Info, "epfd or fd is not a valid file descriptor.");
			return;
		case EEXIST:
			log(Info,
					"was EPOLL_CTL_ADD, and the supplied file descriptor fd is already in epfd.");
			return;
		case EINVAL:
			log(
					Info,
					"epfd is not an epoll file descriptor, or fd is the same as epfd, or the requested operation op is not supported by this interface.");
			return;
		case ENOENT:
			log(Info,
					"op was EPOLL_CTL_MOD or EPOLL_CTL_DEL, and fd is not in epfd. ");
			return;
		case ENOMEM:
			log(Info,
					"There was insufficient memory to handle the requested op control operation.");
			return;
		case EPERM:
			log(Info, "The target file fd does not support epoll.");
			return;
		}
	}
}

void Selector_epoll::select(Socket * s, int remove, int add) {
	//sox::log(Debug, "select begin");
	epoll_event ev;
	ev.data.ptr = s;
	ev.events = EPOLLIN;

	std::pair<sockSet_t::iterator, bool> p = sockets.insert(s);
	if (p.second) {
		if (SEL_WRITE & add) {
			ev.events |= EPOLLOUT;
		}
		//sox::log(Debug, "epoll select add event:", s->socket().getsocket());
		setupEpoll(epfd, EPOLL_CTL_ADD, s->socket().getsocket(), ev);
	} else {
		if (SEL_READ & remove) {
			//sox::log(Debug, "epoll select remove read:", s->socket().getsocket());
			setupEpoll(epfd, EPOLL_CTL_MOD, s->socket().getsocket(), ev);
		}

		if (SEL_WRITE & remove) {
			//sox::log(Debug, "epoll select remove write:", s->socket().getsocket());
			setupEpoll(epfd, EPOLL_CTL_MOD, s->socket().getsocket(), ev);
		}

		if (SEL_READ & add) {
			//sox::log(Debug, "epoll select add read:", s->socket().getsocket());
			setupEpoll(epfd, EPOLL_CTL_MOD, s->socket().getsocket(), ev);
		}

		if (SEL_WRITE & add) {
			//sox::log(Debug, "epoll select add write:", s->socket().getsocket());
			ev.events = EPOLLOUT;
			setupEpoll(epfd, EPOLL_CTL_MOD, s->socket().getsocket(), ev);
		}
	}
	//sox::log(Debug, "select end, events is:", ev.events);
}

void Selector_epoll::mainloop() {
	epoll_event events[EPOLL_SIZE];

	uint64_t oldTimeMs = 0;

	int ptimes = pipe_err;
	while (isRunning()) 
	{
		// at least can receive all error
		//	sox::log(Debug, "epoll loop 1");

		//========= 计算时间 ==============//
		time_t oldtime = sox::env::now;
		struct timeval stv;
		
		gettimeofday(&stv, NULL);
		sox::env::now = stv.tv_sec;
		if(oldtime != sox::env::now)
		{
			char *st = ctime(&sox::env::now);
			sox::env::strTime = std::string(st, strlen(st) - 1);
		}

 		sox::env::haomiao = ((uint64_t)stv.tv_sec) * 1000 + stv.tv_usec/1000;

		if( (sox::env::haomiao -oldTimeMs)  >  (Countdown::TIME_CLICK/10))  //100 毫秒计一次时间 
		{
			if (sox::env::selector())
			{
				sox::env::selector()->increase_elapsed((sox::env::haomiao -oldTimeMs));

				check_timout_run(); // check timeout
			}
			oldTimeMs = sox::env::haomiao;
		}
		//========= 计算时间 ==============//
		
		
		int waits = epoll_wait(epfd, events, EPOLL_SIZE, Countdown::TIME_CLICK);
//		sox::log(Debug, "epoll loop 2 wait:", waits);

		
		if(ptimes != pipe_err){
			ptimes = pipe_err;
			log(Error, "broken pipe times:%u", pipe_err);
		}

		if (waits < 0) 
		{
			if (EINTR == errno) 
			{

				if( m_time_click_callback ){
					m_time_click_callback->signalUser1();
				}

				continue;
			} else 
			{
				log(Info, "epoll error wait:%u str:%s errno:%u str:%s", waits, strerror(waits), errno, strerror(errno));
				//throw exception_errno("epoll");
			}
		}

		if (waits == EPOLL_SIZE) {
			log(Warn, "epoll reach the max size:%d", 100);
		}

		for (int i = 0; i < waits; ++i) {
			//sox::log(Debug, "event is:", events[i].events);
			Socket *sk = (Socket *)events[i].data.ptr;
			if (events[i].events & (EPOLLIN|EPOLLERR|EPOLLHUP)) {
				notify_event(sk, SEL_READ);
			}
			if (events[i].events & EPOLLOUT) {
				notify_event(sk, SEL_WRITE);
			}
		}
		
		interrupt();
		clearRemoved();
		if( m_time_click_callback ){
			m_time_click_callback->timeclick();
		}
	}
}

void Selector_epoll::remove(Socket * s) {
	//sox::log(Debug, "epoll remove begin socket:", int(s), " fd", s->socket().getsocket());
	epoll_event ev;
	setupEpoll(epfd, EPOLL_CTL_DEL, s->socket().getsocket(), ev);
	sockets.erase(s);
	//sox::log(Debug, "remove end");
}
