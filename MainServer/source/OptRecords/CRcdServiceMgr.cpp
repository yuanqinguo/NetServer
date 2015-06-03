#include "CRcdServiceMgr.h"

Locker CRcdServiceMgr::m_locker;
CRcdServiceMgr* CRcdServiceMgr::m_pInstance = NULL;

CRcdServiceMgr::~CRcdServiceMgr()
{
	OnStop();
}

CRcdServiceMgr* CRcdServiceMgr::GetInstance()
{
	if (NULL == m_pInstance)
	{
		LockerGuard guard(m_locker);
		if (NULL == m_pInstance)
		{
			m_pInstance = new CRcdServiceMgr();
		}
	}

	return m_pInstance;
}

void CRcdServiceMgr::DelInstance()
{
	if(m_pInstance)
		delete m_pInstance;
	m_pInstance = NULL;
}

void CRcdServiceMgr::OnStop()
{
	m_isStop = true;
	if(m_pCondition)
	{
		m_pCondition->signalAll();
		delete m_pCondition;
		m_pCondition = NULL;
	}

	pthread_join(m_pthreadid, NULL);

	CRcdLogClient* pClient;
	std::map<int, CRcdLogClient*>::iterator it;
	for (it=m_rcdLogClientMap.begin(); it != m_rcdLogClientMap.end(); )
	{
		pClient = it->second;
		if(pClient)
		{
			pClient->OnStop();
			delete pClient;
			pClient = NULL;
			m_rcdLogClientMap.erase(it++);
		}
		else
		{
			it++;
		}
	}
	m_rcdLogClientMap.clear();
}

bool CRcdServiceMgr::OnStart()
{
	bool isSuccess = OnInit();
	if (!isSuccess)
	{
		WLogError("CRcdServiceMgr::OnStart::Error!\n");
	}
	WLogError("CRcdServiceMgr::OnStart::Success!\n");

	//心跳线程，检测CRcdLogClient的心跳
	m_pCondition = new Condition(m_threadLocker);
	m_isStop = false;
	pthread_create(&m_pthreadid, NULL, ThreadFun, this);
	return isSuccess;
}

bool CRcdServiceMgr::OnInit()
{
	m_pConfiger = CConfiger::GetInstance();
	m_pConfiger->GetConfigInfos(m_RedisConfigs, true);
	m_pConfiger->GetConfigInfos(m_MysqlConfigs);

	if (m_RedisConfigs.size() != m_MysqlConfigs.size())
	{
		WLogError("CRcdServiceMgr::OnInit::Config info error, mysql size != redis size\n");
		return false;
	}

	CRcdLogClient* pClient = NULL;
	std::map<int, ConfigInfo>::iterator it;
	std::map<int, ConfigInfo>::iterator it2;
	for (it=m_MysqlConfigs.begin(), it2=m_RedisConfigs.begin(); 
		it != m_MysqlConfigs.end()&& it2 != m_RedisConfigs.end(); it++, it2++)
	{
		if (it->first != it2->first)
		{
			WLogError("CRcdServiceMgr::OnInit::Config info error, mysql AreaCode =%d != redis AreaCode=%d\n", it->first, it2->first);
			return false;
		}
		pClient = MakeRcdLogClient(it->first, it->second, it2->second);
		if(pClient)
		{
			m_rcdLogClientMap.insert(std::make_pair(it->first, pClient));

			pClient->OnStart();
		}
	}

	return true;
}

void* CRcdServiceMgr::ThreadFun(void* arg)
{
	CRcdServiceMgr* pThisObj = (CRcdServiceMgr*)arg;
	if(pThisObj)
		pThisObj->OnThreadWorker();
	return NULL;
}

void CRcdServiceMgr::OnThreadWorker()
{
	CRcdLogClient* pClient = NULL;
	while(!m_isStop)
	{
		m_threadLocker.lock();
		m_pCondition->waitForSeconds(WAIT_SECOND);

		std::map<int, CRcdLogClient*>::iterator it;
		for (it=m_rcdLogClientMap.begin(); it != m_rcdLogClientMap.end(); )
		{
			pClient = it->second;
			if(pClient)
			{
				if(pClient->OnTestOnline())
				{
					it++;
				}
				else
				{//测试心跳失败，内部已重试三次，故此处直接剔除并记录日志
					WLogError("CRcdServiceMgr::OnThreadWorker::OnTestOnline error,AreaCode =%d\n", it->first);
					pClient->OnStop();
					delete pClient;
					pClient = NULL;
									
					m_rcdLogClientMap.erase(it++);
				}
			}
			else
			{
				it++;
			}
		}
		m_threadLocker.unlock();
	}
}

CRcdLogClient* CRcdServiceMgr::MakeRcdLogClient(int iareaCode, ConfigInfo& mysqlInfo, ConfigInfo& redisInfo)
{
	CRcdLogClient* pClient = NULL;
	pClient = new CRcdLogClient();
	if (pClient)
	{
		char buf[32] = {0};
		sprintf(buf, "%d", iareaCode);
		std::string code = std::string(buf);
		if(!pClient->OnInit(code, mysqlInfo, redisInfo))
		{
			WLogError("CRcdServiceMgr::MakeRcdLogClient::OnInit error\n");
			delete pClient;
			pClient = NULL;
		}
	}
	return pClient;
}