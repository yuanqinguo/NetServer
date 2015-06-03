#include "CRedisServiceMgr.h"
#include "wLog.h"
#include "CConfiger.h"
#include "CRedisClient.h"

#ifndef WAIT_SECOND
#define WAIT_SECOND 5
#endif

Locker CRedisServiceMgr::m_locker;
CRedisServiceMgr* CRedisServiceMgr::m_pInstance = NULL;

CRedisServiceMgr::CRedisServiceMgr()
{
	m_isStop = false;
}

CRedisServiceMgr::~CRedisServiceMgr()
{
	m_isStop = true;
	m_Condition.signalAll();
	//pthread_join(m_pid, NULL);

	LockerGuard guard(m_redisCLocker);
	std::map<int, CRedisClient*>::iterator it;
	for (it=m_RedisClientMap.begin(); it != m_RedisClientMap.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
		}
	}
	m_RedisClientMap.clear();
}
CRedisServiceMgr* CRedisServiceMgr::GetInstance()
{
	if (NULL == m_pInstance)
	{
		LockerGuard guard(m_locker);
		if (NULL == m_pInstance)
		{
			m_pInstance = new CRedisServiceMgr();
		}
	}

	return m_pInstance;
}

void CRedisServiceMgr::DelInstance()
{
	if(m_pInstance)
		delete m_pInstance;
	m_pInstance = NULL;
}

bool CRedisServiceMgr::OnInit()
{
	std::map<int, ConfigInfo> configs;
	std::map<int, ConfigInfo>::iterator it;

	CConfiger::GetInstance()->GetConfigInfos(configs, true);
	for (it=configs.begin(); it!=configs.end(); it++)
	{
		ConfigInfo info;
		info.st_address = it->second.st_address;
		info.st_port = it->second.st_port;
		info.st_password = it->second.st_password;
		info.st_username = it->second.st_username;
		CRedisClient* pClient = MakeRedisClient(&info);
		if(pClient)
			InsertRedisClient(it->first, pClient);
	}

	//启动链接维护线程
	pthread_create(&m_pid, NULL, TestOnLineThreadFun, this);

	return true;
}

//redisClient链接维护线程
void* CRedisServiceMgr::TestOnLineThreadFun(void* arg)
{
	CRedisServiceMgr* pThisObj = (CRedisServiceMgr*)arg;
	if (pThisObj)
	{
		pThisObj->OnTestOnLine();
	}

	return NULL;
}

void CRedisServiceMgr::OnTestOnLine()
{
	while (!m_isStop)
	{
		m_Condition.waitForSeconds(WAIT_SECOND);
		{
			LockerGuard guard(m_redisCLocker);
			std::map<int, CRedisClient*>::iterator it;
			for (it=m_RedisClientMap.begin(); it != m_RedisClientMap.end(); )
			{
				if (it->second)
				{
					if(it->second->TestOnline() != true)
						m_RedisClientMap.erase(it++);
					else
					{
						it++;
					}
				}
				else
				{
					it++;
				}
			}
		}
	}
}

CRedisClient* CRedisServiceMgr::MakeRedisClient(ConfigInfo* pInfo)
{
	CRedisClient* pClient = new CRedisClient();
	if(pClient)
	{
		int port = atoi(pInfo->st_port.c_str());
		if(pClient->OnInit(pInfo->st_address, port))
			return pClient;
		else
		{
			WLogError("CRedisClient* CRedisServiceMgr::MakeRedisClient::OnInit error!, address = %s, port=%d\n",
				pInfo->st_address.c_str(), port);
		}
		delete pClient;
		pClient = NULL;
	}
	return NULL;
}

void CRedisServiceMgr::InsertRedisClient(int iZoneCode, CRedisClient* pClient)
{
	LockerGuard guard(m_redisCLocker);
	m_RedisClientMap.insert(std::make_pair(iZoneCode, pClient));
}

CRedisClient* CRedisServiceMgr::GetRedisClient(int iZoneCode)
{
	CRedisClient* pClient = NULL;
	{
		LockerGuard guard(m_redisCLocker);
		std::map<int, CRedisClient*>::iterator it;
		it = m_RedisClientMap.find(iZoneCode);
		if (it != m_RedisClientMap.end())
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
		CRedisClient* pTmpClient = MakeRedisClient(&info);
		InsertRedisClient(iZoneCode, pTmpClient);
		pClient = pTmpClient;
	}

	return pClient;
}
