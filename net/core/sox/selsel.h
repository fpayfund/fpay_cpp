
#ifndef __SOX_SEL_SEL_H_INCLUDE__
#define __SOX_SEL_SEL_H_INCLUDE__

#include "snox.h"
#include "logger.h"

// define HAVE_XXX or WIN32 before include this.

//#define HAVE_SELECT 
//#define HAVE_KQUEUE
//#define HAVE_POLL // TODO

#if defined (HAVE_SELECT) || defined(WIN32)

#include "selector_sel.h"

inline void sox_env_initialize()
{
	//log(Debug, "HAVE_SEL");
	sox::Sockethelper::initialize();
	sox::env::selector(new sox::Selector_sel());
}

#else

#ifdef HAVE_KQUEUE
#include "selector_kq.h"

inline void sox_env_initialize()
{
	log(Debug, "HAVE_KQUEUE");
	sox::Sockethelper::initialize();
	sox::env::selector(new sox::Selector_kq());
}

#else

#ifdef HAVE_POLL
#include "selector_poll.h"

inline void sox_env_initialize()
{
	log(Debug, "HAVE_POLL");
	sox::Sockethelper::initialize();
	sox::env::selector(new sox::Selector_poll());
}

#else

#ifdef HAVE_EPOLL
#include "Selector_epoll.h"

inline void sox_env_initialize()
{
	log(Info, "HAVE_EPOLL");
	sox::Sockethelper::initialize();
	sox::env::selector(new sox::Selector_epoll());
}


#else

#ifdef HAVE_NU_EPOLL
#include "nuselector_epoll.h"
inline void sox_env_initialize()
{
	        log(Debug, "HAVE_NU_EPOLL");
	        sox::Sockethelper::initialize();
	        sox::env::selector(new sox::NuSelectorEpoll());
}

#else

#error ERROR, NO SELECTOR

#endif /* HAVE_NU_EPOLL */

#endif /* HAVE_EPOLL */

#endif /* HAVE_POLL */

#endif /* HAVE_KQUEUE */

#endif /* HAVE_SELECT || WIN32 */

struct sox_env_help
{
	sox_env_help()  { sox_env_initialize(); }
	~sox_env_help() { sox::env::finalize(); }
};

#endif // __SOX_SEL_SEL_H_INCLUDE__
