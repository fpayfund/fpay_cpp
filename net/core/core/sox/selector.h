
#ifndef __SOX_SELECTOR_H_INCLUDE__
#define __SOX_SELECTOR_H_INCLUDE__

#include "sockethelper.h"
#include "logger.h"
#include "countdown.h"
#include "thrlock.h"

#include <algorithm>



namespace sox
{

	class Handler;
	class Socket;


	struct TimeClickCallbackIf
	{
		virtual void timeclick() {}; 
		virtual void signalUser1() {};
	};


	class Selector
	{
		public:
			virtual ~Selector();
			Selector()  { m_brunning = true; m_elapsed = 0; m_interrupt_handler = NULL; m_time_click_callback = NULL; }
			void stop() { m_brunning = false; }
			bool isRunning() const { return m_brunning; }
			void set_interrupt_handler(Handler * handler);
			virtual void set_max_connections(size_t max) { }

			virtual void remove(Socket * s) { }
			virtual void select(Socket * s, int remove, int add) = 0;
			virtual void mainloop() = 0;
			virtual std::ostream & trace(std::ostream & os) const = 0;

			void register_time_click_callback(TimeClickCallbackIf* If){
				m_time_click_callback = If;
			}

			/* XXX TimeOut check mainloop
			 * WIN32	select + windows timer + thread
			 * freebsd	kqueue + signal(kevent)
			 * other-unix	poll or select + signal
			 */
#ifdef WIN32
			mutable CriticalSection m_cs;
#endif
			void increase_elapsed(int elapsed)
			{
				SCOPED_LOCK();
				m_elapsed += elapsed;
			}

		protected:
			// XXX
			void interrupt() { if (m_interrupt_handler) notify_event(m_interrupt_handler, 0); }

			int get_and_reset_elapsed()
			{
				SCOPED_LOCK();
				int elapsed = m_elapsed;
				m_elapsed = 0;
				return elapsed;
			}
			// check : destroy in loop
			bool isRemoved(void *s) const
			{ return std::find(m_removed.begin(), m_removed.end(), s) != m_removed.end(); }
			void clearRemoved()    { m_removed.clear(); }

			void notify_event(Handler * s, int event);

			void check_timout_run()
			{
				int elapsed = get_and_reset_elapsed();
				if (elapsed > 0)
					timout_run(elapsed);
			}
			void timout_run(int elapsed)
			{
				Countdown::timeout_result result;
				m_countdown.click_elapse(result, elapsed);
				for (Countdown::iterator i=result.begin(); i!=result.end(); ++i)
					notify_event(i->handle(), SEL_TIMEOUT);
				// clearRemoved(); // BUG, can removed only in main loop
			}
			friend class Handler;
			Countdown & countdown() { return m_countdown; }
			void record_removed(void * s) { m_removed.push_back(s); }

			TimeClickCallbackIf* m_time_click_callback;
		private:
			std::vector<void *> m_removed;
			Countdown m_countdown;

			// XXX save elapsed time before check
			int m_elapsed;
			bool m_brunning;
			Handler * m_interrupt_handler;
	};

} // namespace sox

#endif // __SOX_SELECTOR_H_INCLUDE__

