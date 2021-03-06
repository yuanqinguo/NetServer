#include "CMgrRequest.h"
#include "CReqHandlerObj.h"
#include "MemoryPoolObj.h"
#include "locker.h"
#include "wLog.h"


//链接保活时间
#ifndef ACTIVA_TIMEVAL
#define ACTIVA_TIMEVAL 15
#endif

Locker CMgrRequest::m_locker;
CMgrRequest* CMgrRequest::m_pInstance = NULL;

CMgrRequest::CMgrRequest()
{

}

CMgrRequest::~CMgrRequest()
{

}

CMgrRequest* CMgrRequest::GetInstance()
{
	if (NULL == m_pInstance)
	{
		LockerGuard guard(m_locker);
		if (NULL == m_pInstance)
		{
			m_pInstance = new CMgrRequest();
		}
	}

	return m_pInstance;
}

void CMgrRequest::DelInstance()
{
	if(m_pInstance)
		delete m_pInstance;
	m_pInstance = NULL;
}

bool CMgrRequest::ReleaseHandler(evutil_socket_t& sockfd)
{
	CReqHandlerObj* pObj = NULL;
	MemoryPool* pool = MemPoolObj::GetMemoryPool();
	LockerGuard guard(m_LockerReqMap);
	std::map<int, CReqHandlerObj*>::iterator it = m_ReqHandlerMap.find(sockfd);
	if(it!=m_ReqHandlerMap.end())
	{
		pObj = it->second;
		if (pObj)
		{
			EventCtx* pCtx = (EventCtx*)pObj->GetBufferEvent();
			EventCtx_free(pCtx);
			m_ObjMgr.Delete(pool, pObj);
			m_ReqHandlerMap.erase(it);
		}
		return true;
	}

	return false;
}

void CMgrRequest::OnCheckActiveTime()
{
	time_t curTime;
	time(&curTime);
	CReqHandlerObj* pObj = NULL;
	MemoryPool* pool = MemPoolObj::GetMemoryPool();
	LockerGuard guard(m_LockerReqMap);
	std::map<int, CReqHandlerObj*>::iterator it = m_ReqHandlerMap.begin();
	for (it = m_ReqHandlerMap.begin(); it!=m_ReqHandlerMap.end(); )
	{
		pObj = it->second;
		if (pObj)
		{
			int iVal = curTime - pObj->GetActiveTime();
			if (iVal >= ACTIVA_TIMEVAL) //超过ACTIVE_TIMEVAL无数据交互,踢出请求
			{
				EventCtx* pCtx = (EventCtx*)pObj->GetBufferEvent();
				EventCtx_free(pCtx);
				WLogInfo("CMgrRequest::OnCheckActiveTime::EventCtx_free!\n");
				m_ObjMgr.Delete(pool, pObj);
				m_ReqHandlerMap.erase(it++);
				continue;
			}
			it++;
		}
		else
		{
			it++;
		}
	}
}

int CMgrRequest::HandleRequest(EventCtx*& pev, const std::string& request, std::string& reply)
{
	int iRet = -1;
	bool isDone = false;

	CReqHandlerObj* pObj = NULL;
	evutil_socket_t sockfd = pev->sock;
	m_LockerReqMap.lock();
	std::map<int, CReqHandlerObj*>::iterator it;
	it = m_ReqHandlerMap.find(sockfd);
	if (it != m_ReqHandlerMap.end())
	{
		pObj = it->second;
		if(pObj)
		{
			iRet = pObj->CheckData(request);
			if(iRet != DATA_DEF) //数据已接收完成
			{
				isDone = true;
				m_ReqHandlerMap.erase(it);
			}
		}
		m_LockerReqMap.unlock();
	}
	else
	{
		m_LockerReqMap.unlock(); //先解锁，因为内存池内部有锁操作,避免造成死锁
		pObj = m_ObjMgr.Create(MemPoolObj::GetMemoryPool(), pev);
		if(pObj)
		{
			iRet = pObj->CheckData(request);
			if (iRet != DATA_DEF) //数据已接收完成
			{
				isDone = true;	
			}
			else//数据不足，插入队列
			{
				m_LockerReqMap.lock();
				m_ReqHandlerMap.insert(std::make_pair(sockfd, pObj));
				m_LockerReqMap.unlock();
			}
		}
	}

	//数据正确，且接收完成，则处理请求
	if (pObj && 
		isDone && 
		iRet == DATA_OK)
	{
		iRet = pObj->OnHandle(reply);
	}

	if (pObj && iRet != DATA_DEF)
	{
		m_ObjMgr.Delete(MemPoolObj::GetMemoryPool(), pObj);
	}

	return iRet;
}