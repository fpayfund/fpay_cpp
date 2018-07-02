#include <stdio.h>
#include "logger.h"
#include <iostream>
#include <string>
#include <stdarg.h>
#include <ctime>
#ifdef WIN32
#include <tchar.h>
#include <shlobj.h>
#include <windows.h>

#ifdef _DEBUG
#pragma comment(lib,"psapi.lib")
#include <psapi.h>
#endif

#endif

#include <cassert>

#ifndef _countof
#define _countof(array) (sizeof(array)/sizeof(array[0]))
#endif//_countof

#ifndef WIN32

//日志级别控制变量，该变量指向共享内存。该内存的值由其他进程修改。
int *g_pshmLogLevel = NULL;
//检测是否输出日志
#define IS_NOT_DISPLAY_LOG(value)   ( (g_pshmLogLevel != NULL) && (value > *g_pshmLogLevel) )

#endif

#ifdef WIN32
int syslog_level = Debug;//wuji edit
//int syslog_level = -1;//wuji edit
#else
int syslog_level = Info;
#ifdef DYNAMIC_LOG
//log level Initialize Debug
static int  g_LogLevel = Info;
//nomal log level signal handler
void LogLevelNoticeSigHandler(int){
	g_LogLevel = Notice;
    syslog(Notice,"change Log level to Notice.");
}

//debug log level signal handler
void LogLevelDebugSigHandler(int){
	g_LogLevel = Debug;
	syslog(Notice,"change Log level to Debug.");
}

void LogLevelInfoSigHandler(int)
{
	g_LogLevel = Info;
	syslog(Notice, "change log level to Info");
}

//test pass log level is not fit for current log level
#define TEST_LOGGER_LEVEL_INVALUE(iLevel) ((iLevel) > g_LogLevel)
#endif
#endif

const char * levelnames[] = { "debug - ", "info  - ", "WARN  - ", "ERROR - ", "      - " };

const char * LevelToName(int l) {
	return levelnames[l];
}

#ifdef WIN32
#include <fstream>
#include <shlwapi.h>
#include <sys\stat.h> 

//Log file will be truncated during reopening if its size exceeds this value.
const uint64_t N_LOG_SIZE_THRESHOLD=2048000;
const uint64_t N_LOG_CHECK_TIMEOUT=30*60*1000;
const UINT_PTR N_LOG_LIMIT_TIMR_ID=12299;

std::ofstream fout;
std::string strLogFileName;
std::string strLogOldFileName;

std::string safe_time(){
	time_t t = time(NULL);
	char *ct = ctime(&t);
	if(ct){
		size_t len = strlen(ct);
		if(len > 0){
			ct[len - 1] = '\0';
			return std::string(ct, len - 1);
		}
	}
	return "empty time";
}

namespace
{
	std::wstring GetFileDirFromPath(LPCTSTR filepath)
	{
		size_t orgLen = _tcslen(filepath);

		TCHAR end = filepath[ orgLen - 1];

		if(end !=_T('\\') && end != _T(':'))
		{
			LPCTSTR FN = PathFindFileName(filepath);

			return std::wstring(filepath, orgLen -  _tcslen(FN));
		} 
		else
		{
			return std::wstring(filepath);
		}
	}


	std::wstring GetModulePathDir()
	{
		TCHAR	buffer[MAX_PATH];
		ZeroMemory(buffer, sizeof(TCHAR) * MAX_PATH);
		::GetModuleFileName(NULL, buffer, MAX_PATH);

		return GetFileDirFromPath(buffer);
	}

	static std::wstring TransformInstallPath2FolderName( std::wstring const& strPath )
	{
		std::wstring strReturn;
		strReturn.resize(strPath.length());
		for ( size_t i = 0;i < strPath.length();i++ )
		{
			TCHAR c = strPath[i];
			if ( c == _T(':') ||
					c == _T('\\') ||
					c == _T(' ') ||
					c == _T('/') )
			{
				strReturn[i] = _T('_');
			}
			else
			{
				strReturn[i] = strPath[i];
			}
		}
		return strReturn;
	}

	std::wstring GetUserAppDataDir()
	{
		TCHAR buffer[MAX_PATH];
		::ZeroMemory( buffer,MAX_PATH*sizeof(TCHAR) );

		SHGetSpecialFolderPath( NULL,buffer,CSIDL_APPDATA,NULL );

		std::wstring strPath = std::wstring( buffer ) + _T("\\duowan\\yy4.0\\") ;
		strPath += TransformInstallPath2FolderName( GetModulePathDir() );
		strPath += _T("\\");

		int nReturn = SHCreateDirectoryEx( NULL,strPath.c_str(),NULL );

		return strPath;
	}

	std::string toNarrowString( const wchar_t* pStr , int len )
	{
		std::string buf ;

		if(pStr == NULL)
		{
			assert(NULL);
			return buf;
		}

		if (len < 0 && len != -1)
		{
			assert(NULL);
			//OutputDebugStringW( _T("Invalid string length: ") + len);
			return buf;
		}

		// figure out how many narrow characters we are going to get 
		int nChars = WideCharToMultiByte( CP_ACP , 0 , 
			pStr , len , NULL , 0 , NULL , NULL ) ; 
		if ( len == -1 )
			-- nChars ; 
		if ( nChars == 0 )
			return "" ;

		// convert the wide string to a narrow string
		// nb: slightly naughty to write directly into the string like this
		buf.resize( nChars ) ;
		WideCharToMultiByte( CP_ACP , 0 , pStr , len , 
			const_cast<char*>(buf.c_str()) , nChars , NULL , NULL ) ; 

		return buf ; 
	}
	
	uint64_t FileSize64( const char * szFileName ) 
	{ 
		struct __stat64 fileStat; 
		int err = _stat64( szFileName, &fileStat ); 
		if (0 != err) return 0; 
		return fileStat.st_size; 
	}
}

struct FInit {
	FInit() {
		char buffer[MAX_PATH * 2] = {0};
#if 0
		::GetModuleFileNameA(NULL, buffer, MAX_PATH);

		LPCSTR FN = PathFindFileNameA(buffer);

		std::string str(buffer, strlen(buffer) - strlen(FN));
#else
		std::wstring strW = GetUserAppDataDir();
		std::string str = toNarrowString( strW.c_str(),(int)strW.length() );
#endif
#ifdef LOCAL_LOG_DIR
		str = "./"; // Changed by Linyue
#endif
		_snprintf(buffer, _countof(buffer)-1,"%sprotocol.log", str.data());
		strLogFileName = buffer;
		fout.open(buffer, std::ios_base::trunc);
		
		_snprintf(buffer, _countof(buffer)-1,"%sprotocol.old.log", str.data());
		strLogOldFileName = buffer;
		
		fout << "program start: " << safe_time() << std::endl;
	}
	~FInit() {
		fout << "program end:" << safe_time() << std::endl;
	}
};

void ReopenLogFile(std::wstring strNewName)
{
	char buffer[MAX_PATH * 2];
	ZeroMemory(buffer, sizeof(char) * MAX_PATH * 2);
	std::wstring strW = GetUserAppDataDir();
	std::string str = toNarrowString( strW.c_str(),(int)strW.length() );
	_snprintf(buffer, _countof(buffer)-1,"%sprotocol.log", str.data());

#ifdef LOCAL_LOG_DIR
	str = "./"; // Changed by Linyue
#endif
	char buffer2[MAX_PATH * 2] ={0};
	_snprintf(buffer2, _countof(buffer)-1,"%sprotocol.%s.old.log", str.data(),toNarrowString( strNewName.c_str(),(int)strNewName.length()).c_str());	

	strLogOldFileName = buffer2;

	_snprintf(buffer, _countof(buffer)-1,"%sprotocol.%s.log", str.data(),toNarrowString( strNewName.c_str(),(int)strNewName.length()).c_str());

	if (!strLogFileName.compare(buffer)) return; //if equals

	strLogFileName = buffer;

	fout.close();
	
	if (FileSize64(buffer) < N_LOG_SIZE_THRESHOLD)
	{
		fout.open(buffer, std::ios_base::app);	
	}
	else
	{
		remove(buffer2);
		bool bErr = (rename(buffer, buffer2) != 0);
		fout.open(buffer, std::ios_base::trunc);
		if (bErr) fout << "Failed to backup the old log file:" << buffer << std::endl;
	}

	fout << "log reopened: " << safe_time() << std::endl;
#ifdef _DEBUG	
	CTimeMeasure::Init();
	CTimeMeasure objT(__FUNCTION__);
#endif

}

void CALLBACK LoggerTimerProc(HWND hWnd, UINT uMsg, UINT_PTR nTimerID, DWORD dwTime)
{
	fout << "Timer!!!!" << safe_time() << strLogFileName << "  " << strLogOldFileName << "Size:" 
		<< FileSize64(strLogFileName.c_str()) << std::endl;
		
	if (FileSize64(strLogFileName.c_str()) > N_LOG_SIZE_THRESHOLD)
	{
		fout.close();
		remove(strLogOldFileName.c_str());
		bool bErr = (rename(strLogFileName.c_str(), strLogOldFileName.c_str()) != 0);
		fout.open(strLogFileName.c_str(), std::ios_base::trunc);
		if (bErr) 
			fout << "Failed to backup the old log file:" << strLogFileName << std::endl;
		else
			fout << "log rotated: " << safe_time() << std::endl;		
	}
}

struct LogLimiter
{
	LogLimiter() {
		uTid = ::SetTimer(NULL, N_LOG_LIMIT_TIMR_ID, N_LOG_CHECK_TIMEOUT, LoggerTimerProc);
		fout << "LogLimiter start: " << safe_time() << " Tid:" << (int)uTid <<std::endl;
	}
	~LogLimiter() {
		fout << "LogLimiter end:" << safe_time() << " Tid:" << (int)uTid << std::endl;
	}
	
	UINT_PTR uTid;
};

static long __log_lock = 0;

void log(int l, const char *fmt, ...) {
	static FInit init;
	static LogLimiter Limiter;

	va_list		param;

	va_start(param, fmt);
	while (true) {
		if(InterlockedIncrement(&__log_lock) == 1) break;
		InterlockedDecrement(&__log_lock);
	}

	if(syslog_level < l){
		char buf[4096];
		_vsnprintf(buf, 4096, fmt, param);
		fout << safe_time() << ":" << LevelToName(l) << buf << std::endl;
		fout.flush();
	}
	
	InterlockedDecrement(&__log_lock);
	va_end(param);
}

//带函数名字的日志输出
void FunLog::logWithFunName(int  error, const char *fmt, ...)
{
	std::string strLog = std::string("[") + m_functionName + "] "  + fmt;
	char buf[4096];	
	va_list		param;
	va_start(param, fmt);
	_vsnprintf(buf, 4096, strLog.c_str(), param);
	va_end(param);
	log(error, buf);
}
#else

void initLog(){
	openlog(NULL, LOG_PID, LOG_LOCAL0);
}

void log(int l, const char *fmt, ...){
 
	//add log level control
#ifndef  WIN32
	if (IS_NOT_DISPLAY_LOG(l))	{
		return;
	}
	// 浪费太多系统调用
//	if(access("./nolog.txt", F_OK) == 0 && l > Notice){
//		return;
//	}
#endif

#ifdef DYNAMIC_LOG
	if (TEST_LOGGER_LEVEL_INVALUE(l))
	{
		return;
	}
#endif
        		
	va_list		param;
		
	va_start(param, fmt);
	vsyslog(l, fmt, param);
	va_end(param);
}

//带函数名字的日志输出
void FunLog::logWithFunName(int  error, const char *fmt, ...)
{
	std::string strLog = std::string("[") + m_functionName + "] "  + fmt;
	va_list		param;
	va_start(param, fmt);
	vsyslog(error, strLog.c_str(), param);
	va_end(param);
}
#endif
char *ip2str(uint32_t ip)
{
	union ip_addr{
		uint32_t addr;
		uint8_t s[4];
	} a;
	a.addr = ip;
	static char s[16];
	sprintf(s, "%u.%u.%u.%u", a.s[0], a.s[1], a.s[2], a.s[3]);
	return s;
}

std::string ip2string(uint32_t ip)
{
	union ip_addr{
		uint32_t addr;
		uint8_t s[4];
	} a;
	a.addr = ip;
	static char s[16];
	sprintf(s, "%u.%u.%u.%u", a.s[0], a.s[1], a.s[2], a.s[3]);
	return s;
}

#include <sstream>
std::string bin2hex(const char *bin, uint32_t len){
	std::ostringstream os;
	for(uint32_t i = 0; i<len; i++){
		char st[4];
		uint8_t c = bin[i];
		sprintf(st, "%02x ", c);
		os << st;
	}
	return os.str();
}
char *uri2str(uint32_t uri)
{
	uint32_t uri_h = uri/256;
	uint32_t uri_t = uri % 256;

	static char s[16];
	sprintf(s, "%3d|%3d", uri_h, uri_t);
	return s;
}

char *int2str(uint32_t num)
{
	static char s[12];
	sprintf(s, "%u", num);
	return s;
}

std::string int2string(uint32_t num)
{
	char s[12];
	sprintf(s, "%u", num);
	return s;
}


char *int642str(uint64_t num)
{
	static char s[22];
#if defined(__x86_64__)
	sprintf(s, "%ld", num);
#else
	sprintf(s, "%lld", num);
#endif
	return s;
}

#ifdef WIN32

CTimeMeasure::CTimeMeasure(const char *strFunc, bool bAlsoMemInfo)
{
#ifdef _DEBUG
	m_uStart = GetTickCount();
	m_strFunc = strFunc;
	//log(Info, "[%s] **** Timing start ****", m_strFunc);
	if (bAlsoMemInfo)
	{
		PROCESS_MEMORY_COUNTERS ctr;
		//SetProcessWorkingSetSize(m_pProcess, -1, -1); //挤一挤...会不会准确一些?
		GetProcessMemoryInfo(m_pProcess, &ctr, sizeof(ctr));
  		//GetProcessMemoryInfo(m_pProcess == NULL?(m_pProcess = GetCurrentProcess()):m_pProcess, 
  		//	&ctr, sizeof(ctr));
		m_objMemInfo.WorkingSetSize = ctr.WorkingSetSize;
		m_objMemInfo.PeakWorkingSetSize = ctr.PeakWorkingSetSize;
		m_objMemInfo.PagefileUsage = ctr.PagefileUsage;
	}
	else
	{
		m_objMemInfo.cb = 0;
	}	
#endif	
}

#ifdef _DEBUG
void *CTimeMeasure::m_pProcess;

//WorkingSet就是任务管理器中看到的"内存使用“值。但这个其实并不是程序真正使用的物理内存...因此这个函数得到的差值
//也许并不能真正体现出物理内存的消耗变化。
//另外，这个类第一次实例化也有可能造成workingset的增长，例如
//	{CTimeMeasure objT2("pass1");}  //会显示workingset发生变化
//	{CTimeMeasure objT3("pass2");}  //这次就不会了
//所以现在在ReopenLogFile中尝试初始化
CTimeMeasure::~CTimeMeasure()
{
	std::ostringstream strTmp;
	
	strTmp.imbue( std::locale("") );
	
	strTmp << "  *** Time consumption: " << (GetTickCount()-m_uStart) << " ms for ["  << m_strFunc << "]";
	
	if (m_objMemInfo.cb)
	{
		PROCESS_MEMORY_COUNTERS ctr;
		//SetProcessWorkingSetSize(m_pProcess, -1, -1);
		GetProcessMemoryInfo(m_pProcess, &ctr, sizeof(ctr));		

		strTmp << std::endl << " *** PhyMem:     (Delta:" << (signed)(ctr.WorkingSetSize - m_objMemInfo.WorkingSetSize) << ")"
			<< (unsigned int)m_objMemInfo.WorkingSetSize << "->" << (unsigned int)ctr.WorkingSetSize;

		//strTmp << std::endl << " *** PeakPhyMem: (Delta:" << (signed)(ctr.PeakWorkingSetSize - m_objMemInfo.PeakWorkingSetSize) << ")"
			//<< m_objMemInfo.PeakWorkingSetSize << "->" << ctr.PeakWorkingSetSize;

		strTmp << std::endl << " *** VirMem:     (Delta:" << (signed)(ctr.PagefileUsage - m_objMemInfo.PagefileUsage) << ")"
			<< (unsigned int)m_objMemInfo.PagefileUsage << "->" << (unsigned int)ctr.PagefileUsage;
	}
	
	log (Info, strTmp.str().c_str());

/*	
	if (m_objMemInfo.cb)
	{
		PROCESS_MEMORY_COUNTERS ctr;
		GetProcessMemoryInfo(m_pProcess == NULL?(m_pProcess = GetCurrentProcess()):m_pProcess, 
			&ctr, sizeof(ctr));

		log(Info, "  *** Time consumption: %d ms for [%s]. PhyMem: %d -> %d (Delta:%d), PeakPhyMem: %d -> %d (Delta:%d), VMem: %d -> %d (Delta:%d)", 
			GetTickCount()-m_uStart, m_strFunc,
			m_objMemInfo.WorkingSetSize, ctr.WorkingSetSize, ctr.WorkingSetSize - m_objMemInfo.WorkingSetSize,
			m_objMemInfo.PeakWorkingSetSize, ctr.PeakWorkingSetSize, ctr.PeakWorkingSetSize - m_objMemInfo.PeakWorkingSetSize,
			m_objMemInfo.PagefileUsage, ctr.PagefileUsage, ctr.PagefileUsage - m_objMemInfo.PagefileUsage
			);
	}
	else
		log(Info, "  *** Time consumption: %d ms for [%s]", GetTickCount()-m_uStart, m_strFunc);
*/		
}

void CTimeMeasure::Init()
{
	m_pProcess = GetCurrentProcess();
}
#endif	

#endif	
