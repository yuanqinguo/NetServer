#ifndef CREDIS_SERVICE_MGR_H
#define CREDIS_SERVICE_MGR_H

#include "locker.h"
#include "CommStruct.h"
#include <map>

class CRedisClient;
class CConfiger;
class Locker;

/*
* RedisClient 管理对象
*/
class CRedisServiceMgr
{
private:
	CRedisServiceMgr();

public:

	static CRedisServiceMgr* GetInstance();

	static void DelInstance();

	~CRedisServiceMgr();

	bool OnInit();

	//根据区号，获取对应的Client对象
	CRedisClient* GetRedisClient(int iZoneCode);

protected:
	void InsertRedisClient(int iZoneCode, CRedisClient* pClient);
	CRedisClient* MakeRedisClient(ConfigInfo* pInfo);

	static void* TestOnLineThreadFun(void* arg);
	void OnTestOnLine();
private:
	static CRedisServiceMgr* m_pInstance;
	static Locker m_locker;

	Locker m_redisCLocker;
	std::map<int, CRedisClient*> m_RedisClientMap;

	pthread_t m_pid;
	bool m_isStop;
	Cond m_Condition;
};

#endif