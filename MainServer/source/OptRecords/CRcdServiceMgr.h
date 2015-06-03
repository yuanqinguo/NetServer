#ifndef CRCDSERVICEMGR_H
#define CRCDSERVICEMGR_H

#include "CRcdLogClient.h"
#include "CConfiger.h"
#include "Common.h"
#include "locker.h"
#include "wLog.h"

#include <map>
#include <cstring> 

#define TYPE_REDIS 0
#define TYPE_MYSQL 1

class CRcdServiceMgr
{
public:
	~CRcdServiceMgr();

	static CRcdServiceMgr* GetInstance();

	static void DelInstance();

	bool OnStart();

protected:
	bool OnInit();
	void OnStop();

	CRcdLogClient* MakeRcdLogClient(int iareaCode, ConfigInfo& mysqlInfo, ConfigInfo& redisInfo);

	static void* ThreadFun(void* arg);
	void OnThreadWorker();
private:
	CRcdServiceMgr() {	}

	static CRcdServiceMgr* m_pInstance;
	static Locker m_locker;

	std::map<int, CRcdLogClient*> m_rcdLogClientMap;
	std::map<int, ConfigInfo> m_RedisConfigs;
	std::map<int, ConfigInfo> m_MysqlConfigs;

	CConfiger* m_pConfiger;

	pthread_t m_pthreadid;
	Locker m_threadLocker;
	Condition* m_pCondition;
	bool m_isStop;
};

#endif //CRCDSERVICEMGR_H

