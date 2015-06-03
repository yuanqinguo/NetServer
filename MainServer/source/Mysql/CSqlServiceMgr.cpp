#include "CSqlServiceMgr.h"
#include "CConfiger.h"
#include "locker.h"
#include "wLog.h"

#ifndef WAIT_SECOND
#define WAIT_SECOND 5
#endif

Locker CSqlServiceMgr::m_locker;
CSqlServiceMgr* CSqlServiceMgr::m_pInstance = NULL;

CSqlServiceMgr::CSqlServiceMgr()
{
	m_isStop = false;
}

CSqlServiceMgr::~CSqlServiceMgr()
{
	m_isStop = true;
	m_Condition.signalAll();
	//pthread_join(m_pid, NULL);

	LockerGuard guard(m_mysqlCLocker);
	std::map<int, CSqlClient*>::iterator it;
	for (it=m_MysqlClientMap.begin(); it != m_MysqlClientMap.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	m_MysqlClientMap.clear();
}

CSqlServiceMgr* CSqlServiceMgr::GetInstance()
{
	if (NULL == m_pInstance)
	{
		LockerGuard guard(m_locker);
		if (NULL == m_pInstance)
		{
			m_pInstance = new CSqlServiceMgr();
		}
	}

	return m_pInstance;
}

void CSqlServiceMgr::DelInstance()
{
	if(m_pInstance)
		delete m_pInstance;
	m_pInstance = NULL;
}

bool CSqlServiceMgr::OnInit()
{
	std::map<int, ConfigInfo> configs;
	std::map<int, ConfigInfo>::iterator it;

	CConfiger::GetInstance()->GetConfigInfos(configs, false);
	for (it=configs.begin(); it!=configs.end(); it++)
	{
		ConfigInfo info;
		info.st_address = it->second.st_address;
		info.st_port = it->second.st_port;
		info.st_password = it->second.st_password;
		info.st_username = it->second.st_username;
		info.st_dbname = it->second.st_dbname;
		CSqlClient* pClient = MakeSqlClient(&info);
		if(pClient)
			InsertSqlClient(it->first, pClient);
	}

	//启动链接维护线程
	pthread_create(&m_pid, NULL, TestOnLineThreadFun, this);

	return true;
}

//sqlClient链接维护线程
void* CSqlServiceMgr::TestOnLineThreadFun(void* arg)
{
	CSqlServiceMgr* pThisObj = (CSqlServiceMgr*)arg;
	if (pThisObj)
	{
		pThisObj->OnTestOnLine();
	}

	return NULL;
}

void CSqlServiceMgr::OnTestOnLine()
{
	while (!m_isStop)
	{
		m_Condition.waitForSeconds(WAIT_SECOND);
		{
			LockerGuard guard(m_mysqlCLocker);
			std::map<int, CSqlClient*>::iterator it;
			for (it=m_MysqlClientMap.begin(); it != m_MysqlClientMap.end();)
			{
				if (it->second)
				{
					if(it->second->TestOnline() != true)
						m_MysqlClientMap.erase(it++);
					else
						it++;
				}
				else
					it++;
			}
		}
		
	}
}

CSqlClient* CSqlServiceMgr::GetSqlClient(int iZoneCode)
{
	CSqlClient* pClient = NULL;
	{
		LockerGuard guard(m_mysqlCLocker);
		std::map<int, CSqlClient*>::iterator it;
		it = m_MysqlClientMap.find(iZoneCode);
		if (it != m_MysqlClientMap.end())
		{
			pClient =  it->second;
			return pClient;
		}
	}

	ConfigInfo info;
	CConfiger::GetInstance()->GetConfigInfo(iZoneCode, info, true);
	if (!info.st_address.empty() && 
		!info.st_port.empty())
	{
		CSqlClient* pTmpClient = MakeSqlClient(&info);
		InsertSqlClient(iZoneCode, pTmpClient);
		pClient = pTmpClient;
	}

	return pClient;
}

void CSqlServiceMgr::InsertSqlClient(int iZoneCode, CSqlClient* pClient)
{
	LockerGuard guard(m_mysqlCLocker);
	m_MysqlClientMap.insert(std::make_pair(iZoneCode, pClient));
}

CSqlClient* CSqlServiceMgr::MakeSqlClient(ConfigInfo* pInfo)
{
	CSqlClient* pClient = new CSqlClient();
	if(pClient)
	{
		unsigned long flag = 0;
		unsigned int port = atoi(pInfo->st_port.c_str());
		if(pClient->OnInit(pInfo->st_address, port, 
			pInfo->st_username, pInfo->st_password, 
			pInfo->st_dbname, flag))
		{
			return pClient;
		}
		else
		{
			WLogError("CSqlServiceMgr::MakeSqlClient::OnInit error!, address = %s, port=%d, user=%s, password=%s, dbname=%s\n",
				pInfo->st_address.c_str(), port, pInfo->st_username.c_str(), pInfo->st_password.c_str(), pInfo->st_dbname.c_str());
		}
		delete pClient;
		pClient = NULL;
	}
	return NULL;
}