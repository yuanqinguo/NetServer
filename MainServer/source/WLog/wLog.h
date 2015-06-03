#ifndef __W_LOG_H__
#define __W_LOG_H__

//#define CLOSE_WIRTE_LOG

#define MAX_EVENT_TITLE 32
#define MAX_EVENT_TYPE  4

#define LOGTYPE_INFO    0
#define LOGTYPE_WARN    1
#define LOGTYPE_ERROR   2
#define LOGTYPE_DEBUG   3

#define DEFAULT_MIN_LOGFILE_SIZE     100  //默认最小文件个数
#define DEFAULT_MAX_LOGFILE_SIZE     1000 //默认文件个数
#define DEFAULT_DISC_RESERVE_MBYTES	 512  //默认磁盘保留空间
#define DEFAULT_FILE_SIZE (1024*512)

#define CLEAN_TIME_INTERVAL		(1000*60*1) // 清理时间间隔，毫秒

#ifdef WIN32
typedef __int64 INT64;
#else
typedef long long INT64;
#endif

#ifndef CLOSE_WIRTE_LOG
#define WLogInit(dir, filename, minfiles, maxfiles, reserveMB) \
	__W_LOG::Init(dir, filename, (minfiles), (maxfiles), (reserveMB));

#define WLogInfo(fmt,...)	\
	__W_LOG::WriteLog(LOGTYPE_INFO, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);

#define WLogWarn(fmt,...)	\
	__W_LOG::WriteLog(LOGTYPE_WARN, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);

#define WLogError(fmt,...)	\
	__W_LOG::WriteLog(LOGTYPE_ERROR, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);

#define WLogDebug(fmt,...)	\
	__W_LOG::WriteLog(LOGTYPE_DEBUG, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);

#else

#define WLogInit(dir, filename, minfiles, maxfiles, reserveMB)

#define WLogInfo(fmt,...)

#define WLogWarn(fmt,...)

#define WLogError(fmt,...)

#define WLogDebug(fmt,...)
#endif

namespace __W_LOG
{
	void Init(const char *pDir,const char *pFileName, int iMinFiles, int iMaxFiles, int iReserveMBytes);
	void WriteLog(int iType, const char *pFun, int line, const char *fmt, ...);	
};

#endif

