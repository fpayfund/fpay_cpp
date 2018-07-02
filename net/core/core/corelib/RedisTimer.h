#ifndef __REDISTIMER_H__
#define __REDISTIMER_H__
#include "core/sox/snox.h"

template <typename T>
struct redis_bind_func
{

		typedef bool (T::*callback)();
};


template<typename T, typename redis_bind_func<T>::callback ptr>
class RedisTimer : public sox::Handler
{
public:
		RedisTimer()
		{
				_x = NULL;
				_to = 0;
		}
		RedisTimer(T* x){
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

#endif //__REDISTIMER_H__

