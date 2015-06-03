#include "CReqHandlerObj.h"
#include "CRedisServiceMgr.h"
#include "CSqlServiceMgr.h"
#include "MemoryPoolObj.h"

#include "wLog.h"
#include "Common.h"
#include "CTaskRequest.h"


CReqHandlerObj::CReqHandlerObj(void* arg)
{
	m_pArg = arg;
	m_requestStr.clear();
	m_pRedisMgr = CRedisServiceMgr::GetInstance();
	m_pSqlMgr = CSqlServiceMgr::GetInstance();
}

CReqHandlerObj::~CReqHandlerObj()
{
	m_pArg = NULL;
}




//此函数被执行表示当前为一个有效的请求，并且，数据接收完整无误
int CReqHandlerObj::OnHandle(std::string& reply)
{
	CTaskRequest* pObj = m_ObjMgr.Create(MemPoolObj::GetMemoryPool());

	reply = pObj->DoTask(m_requestStr);	
	if (reply.empty())
	{
		WLogError("CReqHandlerObj::OnHandle::Reply-NULL::error!\n");
		reply = REPLY_FAIL;
	}

	m_ObjMgr.Delete(MemPoolObj::GetMemoryPool(), pObj);

	return SUCCESS;
}

int CReqHandlerObj::CheckData(const std::string& request)
{
	//每次数据流通，更新对象活动时间，避免被检测机制剔除
	UpdateActiveTime();

	//检查包头包尾
	m_requestStr += request;
	const char* pHeader = strstr(m_requestStr.c_str(),"##");
	const char* pEnd = strstr(m_requestStr.c_str(),"\r\n");
	if (pHeader && pEnd)
	{
		return DATA_OK;
	}
	else if(!pHeader)
	{
		return DATA_ERROR;
	}
	else if (!pEnd)
	{
		return DATA_DEF;
	}
	return UNKNOW;
}
