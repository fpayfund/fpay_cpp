
#ifndef __SOX_COUNTER_X_H_INCLUDE__
#define __SOX_COUNTER_X_H_INCLUDE__

#include <map>
#include <iostream>

namespace sox
{

template < class _Key >
class Counterx
{
public:
	typedef std::map<_Key, size_t> container_type;
	typedef typename container_type::key_type key_type;

	size_t get(const key_type & key) const
	{
		typename container_type::const_iterator i = m_counter.find(key);
		if (i != m_counter.end())
			return i->second;
		return 0;
	}
	void inc(const key_type & key) { ++ m_counter[key]; }
	bool inc_if(const key_type & key, size_t limit)
	{
		size_t & c = m_counter[key];
		if (c < limit) { ++ c; return true; }
		return false;
	}
	void dec(const key_type & key)
	{
		typename container_type::iterator i = m_counter.find(key);
		if (i != m_counter.end() && (-- i->second) == 0)
			m_counter.erase(i);
	}
	void clear() { m_counter.clear(); }
	std::ostream & trace(std::ostream & os, char div = '\n') const;

private:
	container_type m_counter;
};

template < class _Key >
std::ostream & Counterx<_Key>::trace(std::ostream & os, char div) const
{
	for (typename container_type::const_iterator
		i = m_counter.begin(); i != m_counter.end(); ++i)
		os << i->second << '\t' << i->first << div;
	return os;
}

struct CounterPeak
{
	void inc() { if (++count > peak) peak = count; }
	void dec() { --count; }

	CounterPeak() { reset(); }
	void reset()  { count = 0; peak = 0; }

	unsigned long count;
	unsigned long peak;
};

} // namespace sox

#endif // __SOX_COUNTER_X_H_INCLUDE__
