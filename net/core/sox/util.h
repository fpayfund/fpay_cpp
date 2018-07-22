
#ifndef __SNOX_TIME_H_INCLUDE_
#define __SNOX_TIME_H_INCLUDE_

#ifdef WIN32

#include <windows.h>

#else

#include <time.h>
#include <sys/time.h>

#endif

namespace sox
{

struct hash_pointer  { size_t operator()(void * p) const { return size_t(p); } };
struct equal_pointer { bool operator()(const void * x, const void * y) { return (x == y); } };

#ifdef WIN32

class Timer
{
public:
	Timer() { m_tick_start = GetTickCount(); }
	// tickcount
	int restart()
	{
		int ticks = GetTickCount();
		int e = ticks - m_tick_start;
		m_tick_start = ticks;
		return e;
	}
	int elapsed()
	{
		int ticks = GetTickCount();
		int e = ticks - m_tick_start;
		return e;
	}

private:
	int m_tick_start;
};

#else

class Time
{
public:
	Time () { current(); }
	void current() { gettimeofday(&m_tv, 0); }
	bool operator < (const Time & o) const
	{
		if (m_tv.tv_sec < o.m_tv.tv_sec) return true;
		return (m_tv.tv_sec == o.m_tv.tv_sec && m_tv.tv_usec < o.m_tv.tv_usec);
	}
	int operator - (const Time & o) const
	{
		return (m_tv.tv_sec - o.m_tv.tv_sec) * 1000 + (m_tv.tv_usec - o.m_tv.tv_usec) / 1000;
	}
private:
	struct timeval m_tv;
};

class Timer
{
public:
	Timer() { }
	int restart()
	{
		Time now;
		int e = now - start;
		start = now;
		return e < 0 ? 0 : e;
	}
	int elapsed()
	{
		Time now;
		int e = now - start;
		return e < 0 ? 0 : e;
	}
private:
	Time start;
};

#endif

} // namespace sox

#endif // __SNOX_TIME_H_INCLUDE_
