#ifndef CRECORDLOGCLIENT_H
#define CRECORDLOGCLIENT_H

#include "CRedisClient.h"
#include "CSqlClient.h"
#include "CConfiger.h"
#include "Common.h"
#include "wLog.h"
#include "locker.h"
#include <stdio.h>
#include <cstring> 

#ifndef WAIT_SECOND
#define WAIT_SECOND 5
#endif


class CRcdLogClient
{
public:
	CRcdLogClient();

	~CRcdLogClient();

	bool OnInit(std::string& areaCode, ConfigInfo& mysqlInfo, ConfigInfo& redisInfo);

	bool OnStart();

	void OnStop();

	static void* ThreadFun(void* arg);
	void OnThreadWorker();

	bool OnTestOnline();

protected:
	void OnKeysInit();
	CRedisClient* MakeRedisClient(std::string& address, int port);

	CSqlClient* MakeSqlClient(ConfigInfo* pInfo);

	int RecordToMysql(std::string& sql);

	void RecordFromRedis(std::string& strInKey, std::string& value);

	int GetRedisCount(std::string& strInKey);

	bool GetSqlHeader(size_t index, std::string& header);

	void ProccessTargetKey(std::string& sql, std::string& strInKey);
private:
	unsigned int m_port;
	std::string m_address;
	std::string m_areaCode;

	int m_iReconnectCount;
	int m_insert_wait;
	int m_insert_count;

	CSqlClient* m_pSqlClient;
	CRedisClient* m_pRedisClient;

	std::vector<std::string> m_keyList;

	bool m_isStop;
	Locker m_locker;
	Condition* m_pCondition;
	pthread_t m_threadid;
};

#endif //CRECORDLOGCLIENT_H

