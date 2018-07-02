/**************************************************************************************
* Template will be more universal, just for channel range instance now (I'm lazy```)
* by Yaphet
***************************************************************************************/
#pragma once
#include <map>
#include <set>
#include <stdint.h>
#include <assert.h>
#include <sstream>

class RANGE_KEY
{
public:
	RANGE_KEY( const RANGE_KEY& _range )
	{
		assert( _range.from <= _range.to );
		this->from = _range.from;
		this->to = _range.to;
	}

	RANGE_KEY( uint32_t _from, uint32_t _to)
	{
		assert( _from <= _to );
		this->from = _from;
		this->to = _to;
	}

	explicit RANGE_KEY( uint32_t sid )
	{
		from = sid;
		to = sid+1;
	}

	RANGE_KEY() : from(0), to(0) {}

	//[from, end)注意左是开区间右是闭区间！所以不能用<=只能是<
	bool operator == (const RANGE_KEY& _key) const
	{
		//有重叠即认为“相等”
		if ( this->from < _key.to || _key.from < this->to )
		{
			return true;
		}
		return false;
	}

	//因为开区间和闭区间的关系，这里需要<=而不只是<
	bool operator < (const RANGE_KEY& _key) const
	{
		return this->to <= _key.from;
	}

	//同上
	bool operator > (const RANGE_KEY& _key) const
	{
		return this->from >= _key.to;
	}

	bool empty()
	{
		return to <= from;
	}

public:
	uint32_t from;
	uint32_t to;
};

class RANGE_VALUE
{
public:
	RANGE_VALUE() : id(0) {}
	explicit RANGE_VALUE(uint64_t _id) : id(_id) {}

	bool operator==(const RANGE_VALUE& _value) const
	{
		if ( this->id == _value.id )
			return true;
		return false;
	}
	bool operator<(const RANGE_VALUE& _value) const
	{
		if ( this->id < _value.id )
			return true;
		return false;
	}
	RANGE_VALUE& operator=(uint64_t _id)
	{
		this->id = _id;
		return *this;
	}

public:
	uint64_t id;
};

#ifdef _USE_DE_OVERLAPPING_MAP_RANGE_
typedef std::map<RANGE_KEY, RANGE_VALUE>	RANGE_MAP;
#else
typedef std::multimap<RANGE_KEY, RANGE_VALUE>	RANGE_MAP;
#endif
typedef RANGE_MAP::iterator					RANGE_MAP_IT;
typedef RANGE_MAP::const_iterator			RANGE_MAP_CONST_IT;

typedef std::map<RANGE_KEY, std::set<uint64_t> > RANGE_MAP_RES;

class ChannelRange
{
public:
	ChannelRange(){}
	~ChannelRange(){}

public:
	inline size_t size() const									{ return m_rangeMap.size(); }
	inline bool empty()											{ return size()==0; }
	inline RANGE_MAP_IT begin()									{ return m_rangeMap.begin(); }
	inline RANGE_MAP_IT end()									{ return m_rangeMap.end(); }
	inline RANGE_MAP_CONST_IT begin() const						{ return m_rangeMap.begin(); }
	inline RANGE_MAP_CONST_IT end() const						{ return m_rangeMap.end(); }
	inline RANGE_MAP_IT find(const RANGE_KEY& _key)				{ return m_rangeMap.find(_key); }
	inline RANGE_MAP_CONST_IT find(const RANGE_KEY& _key) const	{ return m_rangeMap.find(_key); }
	inline RANGE_MAP_IT find(uint32_t sid)						{ return m_rangeMap.find( RANGE_KEY(sid) ); }
	inline RANGE_MAP_CONST_IT find(uint32_t sid) const			{ return m_rangeMap.find( RANGE_KEY(sid) ); }
	inline void erase(RANGE_KEY _key)							{ m_rangeMap.erase(_key); }
	inline void erase(RANGE_MAP_IT it)							{ m_rangeMap.erase(it); }
	inline void erase(uint64_t id)								{ this->erase(RANGE_VALUE(id)); }
	inline void clear()											{ m_rangeMap.clear(); }

	void erase(RANGE_VALUE value)
	{
		for ( RANGE_MAP_IT io = m_rangeMap.begin(); io != m_rangeMap.end(); )
		{
			RANGE_MAP_IT tmp_io = io; ++io;
			if ( value == tmp_io->second )
			{
				m_rangeMap.erase(tmp_io);
			}
		}
	}

	void arrange()
	{
		std::map<RANGE_KEY, std::set<RANGE_VALUE> > _tmpMap;
		for ( RANGE_MAP_CONST_IT io = m_rangeMap.begin(); io != m_rangeMap.end(); ++io )
		{
			_tmpMap[io->first].insert(io->second);
		}
		m_rangeMap.clear();
		for ( std::map<RANGE_KEY, std::set<RANGE_VALUE> >::const_iterator io = _tmpMap.begin(); io != _tmpMap.end(); ++io )
		{
			for ( std::set<RANGE_VALUE>::const_iterator io2 = io->second.begin(); io2 != io->second.end(); ++io2 )
			{
				m_rangeMap.insert( std::make_pair(io->first, *io2) );
			}
		}
	}

	RANGE_MAP_RES insert(uint32_t _from, uint32_t _to, uint64_t _id)
	{
		RANGE_MAP_RES _res;
		this->insert(_from, _to, _id, _res);
		return _res;
	}

	//反回重叠区域的service id集合, map< pair<from,to>, set<serviceId> >, 迭代递归实现,先分割记录重叠区域再作递归
	void insert(uint32_t _from, uint32_t _to, uint64_t _id, RANGE_MAP_RES& _res)
	{
		RANGE_MAP_IT iFind = m_rangeMap.find( RANGE_KEY(_from, _to) );
		if ( m_rangeMap.end() != iFind ) //这是一个迭代的过程，这里不用equal_range，涉及erase，equal_range难以处理
		{
			uint32_t _old_from	= iFind->first.from;
			uint32_t _old_to	= iFind->first.to;
			uint64_t _old_id	= iFind->second.id;
			m_rangeMap.erase(iFind); //删除大块的旧区间，作切割后加回

			//既然命中，就是重叠(根据RANGE_KEY的operator==判定)
			if ( _to <= _old_to )
			{
				if ( _old_from <= _from ) //新加入区间在旧区间内 old_from, _from, _to, old_to，重叠区域[_from, _to) 作切割后[old_from, _from) [_from, _to) [_to, old_to)
				{
					std::set<uint64_t>& _ids = _res[RANGE_KEY(_from, _to)];
					_ids.insert(_old_id);
					_ids.insert(_id);
					//split
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_from, _to), RANGE_VALUE(_old_id) ) ); //overlap
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_from, _to), RANGE_VALUE(_id) ) ); //overlap
					if ( _old_from < _from ) //只有两个区间的开端不一致时才insert[old_from, from)，相等则不加
					{
						this->insert(_old_from, _from, _old_id, _res);
					}
					if ( _to < _old_to ) //只有两个区间的末端不一致时才insert[_to, _old_to)，相等则不加
					{
						this->insert(_to, _old_to, _old_id, _res);
					}
				}
				else //新加入区间有一部分在旧区间左边 _from, old_from, _to, old_to, 重叠区域[old_from, _to) 切割后 [_from, old_from) [old_from, _to) [_to, old_to)
				{
					std::set<uint64_t>& _ids = _res[RANGE_KEY(_old_from, _to)];
					_ids.insert(_old_id);
					_ids.insert(_id);
					//split
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_old_from, _to), RANGE_VALUE(_id) ) );//overlap
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_old_from, _to), RANGE_VALUE(_old_id) ) );//overlap
					this->insert(_from, _old_from, _id, _res);
					if ( _to < _old_to ) //只有两个区间的末端不一致时才insert[_to, _old_to)，相等则不加
					{
						this->insert(_to, _old_to, _old_id, _res);
					}
				}
			}
			else
			{
				if ( _old_from <= _from ) // _old_from, _from, _old_to, _to，重叠区域[_from, _old_to)
				{
					std::set<uint64_t>& _ids = _res[RANGE_KEY(_from, _old_to)];
					_ids.insert(_old_id);
					_ids.insert(_id);
					//split
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_from, _old_to), RANGE_VALUE(_old_id) ) ); //overlap
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_from, _old_to), RANGE_VALUE(_id) ) ); //overlap
					if ( _old_from < _from )
					{
						this->insert(_old_from, _from, _old_id, _res);
					}
					this->insert(_old_to, _to, _id, _res);
				}
				else //_from, _old_from, _old_to, _to， 重叠区域[_old_from, _old_to)
				{
					std::set<uint64_t>& _ids = _res[RANGE_KEY(_old_from, _old_to)];
					_ids.insert(_old_id);
					_ids.insert(_id);
					//split
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_old_from, _old_to), RANGE_VALUE(_id) ) );//overlap
					m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_old_from, _old_to), RANGE_VALUE(_old_id) ) );//overlap
					this->insert(_from, _old_from, _id, _res);
					this->insert(_old_to, _to, _id, _res);
				}
			}
		}
		else//没有重叠区域，直接insert
		{
			m_rangeMap.insert( RANGE_MAP::value_type( RANGE_KEY(_from, _to), RANGE_VALUE(_id) ) );
		}
		//return _res;
	}

#ifndef _USE_DE_OVERLAPPING_MAP_RANGE_
	std::pair<RANGE_MAP_IT, RANGE_MAP_IT> equal_range(const RANGE_KEY& _key)				{ return m_rangeMap.equal_range(_key); }
	std::pair<RANGE_MAP_IT, RANGE_MAP_IT> equal_range(uint32_t sid)							{ return m_rangeMap.equal_range( RANGE_KEY(sid) ); }
	std::pair<RANGE_MAP_CONST_IT, RANGE_MAP_CONST_IT> equal_range(const RANGE_KEY& _key) const { return m_rangeMap.equal_range(_key); }
	std::pair<RANGE_MAP_CONST_IT, RANGE_MAP_CONST_IT> equal_range(uint32_t sid)	const		{ return m_rangeMap.equal_range( RANGE_KEY(sid) ); }
#endif

public:
	std::set<uint64_t> getTargetIdBySid(uint32_t sid)
	{
		std::set<uint64_t> _res;
		std::pair<RANGE_MAP_IT, RANGE_MAP_IT> iFindRange = this->equal_range(sid);
		for ( RANGE_MAP_IT io = iFindRange.first; io != iFindRange.second; ++io )
		{
			_res.insert(io->second.id);
		}
		return _res;
	}

	RANGE_KEY getTargetRangeBySid(uint32_t sid)
	{
		RANGE_KEY _res;
		RANGE_MAP_IT iFind = this->find(sid); //same range, so```use find instead of equal_range
		if ( m_rangeMap.end() != iFind )
		{
			_res = iFind->first;
		}
		return _res;
	}

	RANGE_MAP_RES getTargetBySid(uint32_t sid)
	{
		RANGE_MAP_RES _res;
		std::pair<RANGE_MAP_IT, RANGE_MAP_IT> iFindRange = this->equal_range(sid);
		for ( RANGE_MAP_IT io = iFindRange.first; io != iFindRange.second; ++io )
		{
			_res[io->first].insert(io->second.id);
		}
		return _res;
	}

	RANGE_MAP_RES getSameRangeInfo()
	{
		RANGE_MAP_RES _res;
		for ( RANGE_MAP_CONST_IT io = m_rangeMap.begin(); io != m_rangeMap.end(); ++io )
		{
			_res[io->first].insert(io->second.id);
		}
		return _res;
	}

	void showRange(std::ostringstream& os)
	{
		std::map<RANGE_KEY, std::set<uint64_t> > _tmpMap;
		for ( RANGE_MAP_CONST_IT io = m_rangeMap.begin(); io != m_rangeMap.end(); ++io )
		{
			_tmpMap[io->first].insert(io->second.id);
		}
		for ( std::map<RANGE_KEY, std::set<uint64_t> >::iterator io = _tmpMap.begin(); io != _tmpMap.end(); ++io )
		{
			os << "[" << io->first.from << "," << io->first.to << ")\t";
			for ( std::set<uint64_t>::iterator io2 = io->second.begin(); io2 != io->second.end(); ++io2 )
			{
				os << std::hex << *io2 << std::dec << "\t";
			}
			os << "\r\n";
		}
	}

public:
	RANGE_MAP m_rangeMap;
};

