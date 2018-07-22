
#ifndef __SOX_COUNTDOWN_H_INCLUDE__
#define __SOX_COUNTDOWN_H_INCLUDE__

#include <list>
#include <vector>
#include <iostream>
#include <algorithm>

namespace sox
{

/*
   我很丑，可是我很温柔

1. 对于巨大的数量，但是实际超时值在一定范围的timeout检查，
   可以用hash->list来进一步减少循环. 需要时再说

2. XXX
   splice 的用法不符合c++标准. 可以修改成让 splice 返回在新的列表里面的iterator

*/

class Handler;
class Countdown
{
	class Timeout
	{
		friend class Countdown;
		int fallidx;
		Handler * handler;
		int count;
	public:
		Timeout() { }
		Timeout(int idx, Handler * h, int c) : fallidx(idx), handler(h), count(c) { }
		Handler * handle() { return handler; }
	};
	typedef std::list<Timeout>     _list;
	typedef _list::iterator        _list_iter;
	typedef _list::const_reference param_type;

	_list_iter null_iter() const         { return _list_iter(); }
	bool isRegister(_list_iter _x) const 
	{ 
#if _MSC_VER >= 1400
		return false;
#else
		return _x != null_iter(); 
#endif
	}

public:
	enum { TIME_CLICK = 100 };
	struct iterator : public _list_iter
	{
		iterator() : _list_iter() { }
		iterator(const _list_iter & _x) : _list_iter(_x) { }
		iterator & operator =(const _list_iter & _x) { *((_list_iter*)this) = _x; return *this; }
	};

	Countdown();

	//typedef std::list<Handler*> timeout_result;
	typedef _list timeout_result;
	void click_elapse(timeout_result & result, int elasped);
	void select_timeout(Handler * s, int timeout);
	std::ostream & trace(std::ostream & os, bool dumpall = true) const;

private:
	struct Fall
	{
		Fall() { m_elasped = 0; }
		Fall(int fall) { m_elasped = 0; m_fall = fall; }

		int m_elasped;
		int m_fall;
		_list m_list;

		void erase(iterator i)                { m_list.erase(i); }
		iterator insert(param_type x)   { return m_list.insert(m_list.end(), x); }
		void splice(_list & from, iterator i) { m_list.splice(m_list.end(), from, i); }
	};
	typedef std::vector<Fall> Falls;
	Falls  m_falls;
	size_t m_falls_last_idx;

	enum { npos = size_t(-1) };
	size_t find_fall_idx(int timeout, size_t rbegin = npos)
	{
		size_t i = rbegin; if (m_falls_last_idx < i) i = m_falls_last_idx; // std::min vc XXX
		for ( ; i > 0; --i)
			if (timeout >= m_falls[i].m_fall)
				break;
		return i;
	}
};

} // namespace sox

#endif //  __SOX_COUNTDOWN_H_INCLUDE__
