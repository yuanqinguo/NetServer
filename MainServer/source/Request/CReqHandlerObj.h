#ifndef CREQ_HANDLER_OBJ_H
#define CREQ_HANDLER_OBJ_H

#include <string>
#include "locker.h"
#include "CObjPoolTmpl.h"
#include "CHttpParse.h"

/*
* 请求处理对象，每个请求为一个CRequestObj对象
*/
class CRedisServiceMgr;
class CSqlServiceMgr;
class CTaskRequest;
class CReqHandlerObj
{
public:
	CReqHandlerObj(void* arg);

	~CReqHandlerObj();

	void* GetBufferEvent()
	{
		return m_pArg;
	}

	int CheckData(const std::string& request);

	int OnHandle(std::string& reply);

	time_t GetActiveTime()
	{
		LockerGuard guard(m_locker);
		return m_activeTime;
	}
protected:
	void UpdateActiveTime()
	{
		time_t active;
		time(&active);
		LockerGuard guard(m_locker);
		m_activeTime = active;
	}
private:
	
	int GetMemCacheKey(const char *pAreaCode, const char* pKey, char* pCacheValue);

	std::string m_requestStr;

	Locker m_locker;
	time_t m_activeTime;

	CRedisServiceMgr* m_pRedisMgr;
	CSqlServiceMgr* m_pSqlMgr;
	CHttpParse* m_pHttpParse;

	void* m_pArg;
};
#endif //CREQ_HANDLER_OBJ_H