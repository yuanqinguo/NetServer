#ifndef CSQL_SERVICE_MGR_H
#define CSQL_SERVICE_MGR_H

#include "CommStruct.h"
#include "CSqlClient.h"
#include "locker.h"
#include <map>

class CSqlServiceMgr
{
private:
	CSqlServiceMgr();
public:
	static CSqlServiceMgr* GetInstance();

	static void DelInstance();

	~CSqlServiceMgr();

	bool OnInit();

	CSqlClient* GetSqlClient(int iZoneCode);

protected:
	void InsertSqlClient(int iZoneCode, CSqlClient* pClient);
	CSqlClient* MakeSqlClient(ConfigInfo* pInfo);

	static void* TestOnLineThreadFun(void* arg);
	void OnTestOnLine();
private:
	static CSqlServiceMgr* m_pInstance;
	static Locker m_locker;

	Locker m_mysqlCLocker;
	std::map<int, CSqlClient*> m_MysqlClientMap;

	pthread_t m_pid;
	bool m_isStop;
	Cond m_Condition;
};

#endif