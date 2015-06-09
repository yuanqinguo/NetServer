#ifndef CREQUEST_MGR_H
#define CREQUEST_MGR_H

#include <string>
#include <map>

#include "CEventBaseMgr.h"
#include "event2/util.h"
#include "locker.h"
#include "CommStruct.h"
#include "CObjPoolTmpl.h"


/*
* 请求处理对象管理
*/

class CReqHandlerObj;
class CTaskRequest;

class CMgrRequest
{
private:
	CMgrRequest();

public:
	static CMgrRequest* GetInstance();

	static void DelInstance();

	~CMgrRequest();

	bool ReleaseHandler(evutil_socket_t& sockfd);

	int HandleRequest(EventCtx*& pev, const std::string& request, std::string& reply);

	void OnCheckActiveTime();
protected:
	
private:
	static CMgrRequest* m_pInstance;
	static Locker m_locker;

	Locker m_LockerReqMap;
	std::map<int, CReqHandlerObj*> m_ReqHandlerMap;
	pthread_t m_pid;
	bool m_isStop;
	Cond m_exitCondition;

	ObjectManager<CReqHandlerObj> m_ObjMgr;
};
#endif //CREQUEST_MGR_H