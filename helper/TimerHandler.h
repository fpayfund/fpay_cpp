#ifndef __FPAY_TIME_HANDLER_H__
#define __FPAY_TIME_HANDLER_H__
#include "core/sox/snox.h"

template <typename T>
struct bind_func
{

		typedef bool (T::*callback)();
};


template<typename T, typename bind_func<T>::callback ptr>
class TimerHandler : public sox::Handler
{
public:
		TimerHandler()
		{
				_x = NULL;
				_to = 0;
		}
		TimerHandler(T* x){
				_x = x;
				_to = 0;
		}
		void init(T* x)
		{
				_x = x;
		}
		void start(uint32_t timeout)
		{
				_to = timeout;
				select_timeout(_to);
		}
		void stop()
		{
				select_timeout();
		}
		void handle(int /*ev*/)
		{
				if(_x) 
				{		
						if((_x->*ptr)()) select_timeout(_to);
				}
		}
protected:
		T* _x;
		uint32_t _to;
};

#endif 

