#ifndef MEASHRE_H_
#define MEASHRE_H_
#include <string>

// YYMS 
#ifndef WIN32
#include <sys/time.h>
//typedef uint32_t URI_TYPE;
#define rdtsc(low,high) __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high))

uint64_t get_cpu_cycle();
uint64_t get_usec_interval( uint64_t start, uint64_t stop );
uint32_t get_msec_interval( uint64_t start, uint64_t stop );
uint64_t get_tick_interval(uint64_t start, uint64_t stop);
int init_cpu_freq();

void measureDump(bool getFrom = false, bool getFromIp = false);
void measureDumpToConsole();
std::string measureDumpJSON(bool getFrom = true, bool getFromIp = true, bool getAndClear = false);

class scoped_measure
{
public:
	scoped_measure(URI_TYPE uri, uint32_t ip, uint32_t serverId);
	~scoped_measure();
	
private:
	uint64_t m_startTime;
	URI_TYPE m_uri;
	uint32_t m_ip;
	uint32_t m_serverId;
};

void measureEnableMultiThread();

void measureSetQueueSize(uint32_t n);

void measureSetActiveThread(uint32_t n);
void measureSetDropedRequest(uint32_t n);

#endif
//End of YYMS

#endif /*MEASHRE_H_*/
