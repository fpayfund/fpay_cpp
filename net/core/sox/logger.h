
#ifndef EIMD_LOGGER_H
#define EIMD_LOGGER_H
#include <string>

#ifdef WIN32
enum LogLevel{
	Debug = 0,
	Info,
	Notice,
	Warn,
	Error,
	Fatal
};

#include <string>
void ReopenLogFile(std::wstring strNewName);

#else
#include <syslog.h>
enum LogLevel{
	Fatal = LOG_EMERG,
	Error = LOG_ERR,
	Warn = LOG_WARNING,
	Notice = LOG_NOTICE,
	Info = LOG_INFO,
	Debug = LOG_DEBUG
};
#endif

void initLog();
void log(int  error, const char *fmt, ...);
void LogLevelDebugSigHandler(int);
void LogLevelInfoSigHandler(int);
//helper function, not thread safe
#include "int_types.h"
#include <string>
char *int642str(uint64_t num);
char *int2str(uint32_t num);
std::string int2string(uint32_t i);
char *ip2str(uint32_t ip);
std::string ip2string(uint32_t ip);
char *uri2str(uint32_t uri);
std::string bin2hex(const char *bin, uint32_t len);

//函数内日志的封装
class FunLog
{
public:
	FunLog(const char * functionName, bool bEnterLeaveLog = false):m_functionName(functionName), m_LeaveLogLevel(Debug),m_bEnterLeaveLog(bEnterLeaveLog)
	{
		if(m_bEnterLeaveLog)
		{
			//log(Debug, "[%s]: function enter!", m_functionName);
			logWithFunName(Debug, "function enter!");
		}		
	}

	virtual ~FunLog()
	{
		//输出退出日志
		if(m_bEnterLeaveLog)
		{
			if(m_LeaveLogLevel == Debug)
			{
				//log(m_LeaveLogLevel, "[%s]: function leave normal!");	//正常退出
				logWithFunName(m_LeaveLogLevel, "function leave normal!");
			}
			else
			{
				//log(m_LeaveLogLevel, "[%s]: function leave abnormal!");	//非正常退出
				logWithFunName(m_LeaveLogLevel, "function leave abnormal!");
			}
		}	
	}

	//带函数名字的日志输出
	void logWithFunName(int  error, const char *fmt, ...);

	//设置为非正常退出,当需要函数进入和退出日志且刚好又非正常退出时使用
	void setAbnormalLeave()
	{
		m_LeaveLogLevel= Warn;
	}

private:
	FunLog();
	const char * m_functionName;
	LogLevel m_LeaveLogLevel;		//退出时的日志是标记正常还是错误.
	bool m_bEnterLeaveLog;
};

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////

#ifndef WIN32

#include <stdlib.h>
#include <iostream>
#include <sys/shm.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#ifdef DYNAMIC_LOG
//nomal signal handler
void LogLevelNoticeSigHandler(int);

//debug signal handler
void LogLevelDebugSigHandler(int);
#endif

#define  MEM_SHARE_KEY      (0xFFFFFF)
#define  PERM               (S_IRUSR|S_IWUSR)

extern   int *g_pshmLogLevel ;

//获取或创建共享内存，并连接到共享内存。设置共享内存初始值为Debug
#define  SET_LOG_LEVEL_CONTROL_MULTI_THREAD \
{\
	using namespace std;\
	int shmId = shmget(MEM_SHARE_KEY,sizeof(int),PERM|IPC_CREAT|IPC_EXCL);\
	if ((shmId == -1) && (errno != EEXIST) )\
	{\
		cout<<"error SET_LOG_LEVEL_CONTROL_MULTI_THREAD,shmget  faile! errno = "<<errno<<endl;\
		exit(1);\
	}\
	if(shmId == -1)\
	{\
		if(((shmId = shmget(MEM_SHARE_KEY,sizeof(int),PERM)) == -1) || ((g_pshmLogLevel =(int *) shmat(shmId,NULL,0)) == (void *)-1))\
		{\
			if(shmId != -1)\
			{\
				shmctl(shmId,IPC_RMID,NULL);\
				cout<<"error SET_LOG_LEVEL_CONTROL_MULTI_THREAD,shmat faile! errno = "<<errno<<endl;\
			}\
			else\
			{\
				cout<<"error SET_LOG_LEVEL_CONTROL_MULTI_THREAD,shmget faile! errno = "<<errno<<endl;\
			}\
			exit(1);\
		}\
	}\
	else if((g_pshmLogLevel = (int *)shmat(shmId,NULL,0)) == (void*)-1)\
	{\
		shmctl(shmId,IPC_RMID,NULL);\
		cout<<"error SET_LOG_LEVEL_CONTROL_MULTI_THREAD,shmat faile! errno = "<<errno<<endl;\
		exit(1);\
	}\
	*g_pshmLogLevel = Debug;\
}

//这个宏与上面的宏一致，为了兼容旧代码才保留这个宏。
#define SET_LOG_LEVEL_CONTROL_SINGLE_THREAD   SET_LOG_LEVEL_CONTROL_MULTI_THREAD

	
#endif

#ifdef WIN32
struct CProcessInfo {
	uint32_t cb;
	uint32_t PageFaultCount;
	size_t PeakWorkingSetSize;
	size_t WorkingSetSize;
	size_t QuotaPeakPagedPoolUsage;
	size_t QuotaPagedPoolUsage;
	size_t QuotaPeakNonPagedPoolUsage;
	size_t QuotaNonPagedPoolUsage;
	size_t PagefileUsage;
	size_t PeakPagefileUsage;
};

class CTimeMeasure
{
public:
	CTimeMeasure(const char *strFunc, bool bAlsoMemInfo = true);
#ifdef _DEBUG
	~CTimeMeasure();
	static void Init();
#endif	
private:
#ifdef _DEBUG
	uint32_t m_uStart;
	const char *m_strFunc;
	CProcessInfo m_objMemInfo;
	static void * m_pProcess;	
#endif	
};
#endif
#endif // EIMD_LOGGER_H
