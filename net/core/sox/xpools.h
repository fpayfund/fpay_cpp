
#ifndef __POPO_BOOST_POOL_H_INCLUDE__
#define __POPO_BOOST_POOL_H_INCLUDE__

//#include <boost/pool/detail/singleton.hpp>
#include "Singleton.h"
#include <boost/pool/pool.hpp>

template <unsigned RequestedSize, unsigned NextSize>
class xsingleton_pool // non-thread-safe
{
public:
	typedef typename boost::pool<>::size_type size_type;
	typedef typename boost::pool<>::difference_type difference_type;

	BOOST_STATIC_CONSTANT(unsigned, requested_size = RequestedSize);
	BOOST_STATIC_CONSTANT(unsigned, next_size = NextSize);
private:
	struct pool_type
	{
		boost::pool<> p;
		pool_type():p(RequestedSize, NextSize) { }
	};
	//typedef boost::details::pool::singleton_default<pool_type> singleton;
	typedef singleton_default<pool_type> singleton;
	xsingleton_pool();
public:
	static void * malloc()
	{ return singleton::instance().p.malloc(); }

	static void * ordered_malloc()
	{ return singleton::instance().p.ordered_malloc(); }

	static void free(void * const ptr)
	{ singleton::instance().p.free(ptr); }

	static void ordered_free(void * const ptr)
	{ singleton::instance().p.ordered_free(ptr); }

	static void * ordered_malloc(const size_type n)
	{ return singleton::instance().p.ordered_malloc(n); }

	static void free(void * const ptr, const size_type n)
	{ singleton::instance().p.free(ptr, n); }

	static void ordered_free(void * const ptr, const size_type n)
	{ singleton::instance().p.ordered_free(ptr, n); }

	static bool release_memory()
	{ return singleton::instance().p.release_memory(); }

	static bool purge_memory()
	{ return singleton::instance().p.purge_memory(); }

	static bool is_from(void * const ptr)
	{ return singleton::instance().p.is_from(ptr); }
};

#define DECLARE_OPERATOR_NEW_DELETE                           \
	static void * operator new (size_t size);             \
	static void operator delete(void * raw, size_t size); \

#define IMPLEMENT_OPERATOR_NEW_DELETE(classname, count) \
typedef xsingleton_pool<sizeof(classname), count> static_pool_##classname ; \
inline void * classname::operator new (size_t size) { \
	if (size == sizeof(classname)) \
		return static_pool_##classname::malloc(); \
	return ::operator new(size); \
} \
 \
inline void classname::operator delete(void * raw, size_t size) \
{ \
	if (raw != 0 && size == sizeof(classname)) \
		static_pool_##classname::free(raw); \
	else \
		::operator delete(raw); \
} \

template <typename T>
class xobject_pool // non-thread-safe 
{
public:
	typedef xsingleton_pool<sizeof(T), 32> static_object_pool;

	static T * construct()
	{
		T * ret = static_cast<T *>(static_object_pool::malloc());
		if (ret == 0)
			return ret;
		try { new (ret) T(); }
		catch (...) { static_object_pool::free(ret); throw; }
		return ret;
	}

	template <typename T0>
	static T * construct(const T0 & a0)
	{
		T * const ret = static_cast<T *>(static_object_pool::malloc());
		if (ret == 0)
			return ret;
		try { new (ret) T(a0); }
		catch (...) { static_object_pool::free(ret); throw; }
		return ret;
	}

	template <typename T0, typename T1>
	static T * construct(const T0 & a0, const T1 & a1)
	{
		T * const ret = static_cast<T *>(static_object_pool::malloc());
		if (ret == 0)
			return ret;
		try { new (ret) T(a0, a1); }
		catch (...) { static_object_pool::free(ret); throw; }
		return ret;
	}

	template <typename T0, typename T1, typename T2>
	static T * construct(const T0 & a0, const T1 & a1, const T2 & a2)
	{
		T * const ret = static_cast<T *>(static_object_pool::malloc());
		if (ret == 0)
			return ret;
		try { new (ret) T(a0, a1, a2); }
		catch (...) { static_object_pool::free(ret); throw; }
		return ret;
	}

	static void destroy(T *p)
	{
		p->~T();
		static_object_pool::free(p);
	}
};

#endif // __POPO_STATIC_POOL_H_INCLUDE__

