
#ifndef __SOX_THREAD_LOCK_H_INCLUDE__
#define __SOX_THREAD_LOCK_H_INCLUDE__

// XXX helper .  see boost::thread
#ifdef WIN32

#pragma warning( disable : 4786 )

class CriticalSection
{
public:
	struct scoped_lock
	{
		scoped_lock(CriticalSection * c) { cs = c; cs->Lock(); }
		~scoped_lock() { cs->Unlock(); }
		CriticalSection * cs;
	};

	CriticalSection()  { InitializeCriticalSection(&m_sect); }
	~CriticalSection() { DeleteCriticalSection(&m_sect); }
	void Lock()   { EnterCriticalSection(&m_sect); }
	void Unlock() { LeaveCriticalSection(&m_sect); }

private:
	CRITICAL_SECTION m_sect;
};

#define SCOPED_LOCK() CriticalSection::scoped_lock lock(&m_cs)

#else

#define SCOPED_LOCK()

#endif

#endif // __SOX_THREAD_LOCK_H_INCLUDE__
