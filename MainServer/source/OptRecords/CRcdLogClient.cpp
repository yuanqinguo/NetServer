#include "CRcdLogClient.h"
#include "CUtilRedis.h"
#include "wLog.h"

CRcdLogClient::CRcdLogClient()
{
	m_pSqlClient = NULL;
	m_pRedisClient = NULL;
	m_isStop = false;
	m_pCondition = new Condition(m_locker);

	m_insert_wait = CConfiger::GetInstance()->GetInsertWait();
	m_insert_count = CConfiger::GetInstance()->GetInsertCount();
}

CRcdLogClient::~CRcdLogClient()
{
	if (m_pRedisClient)
	{
		delete m_pRedisClient;
		m_pRedisClient = NULL;
	}
	if (m_pSqlClient)
	{
		delete m_pSqlClient;
		m_pSqlClient = NULL;
	}
	if(m_pCondition)
	{
		delete m_pCondition;
		m_pCondition = NULL;
	}
}

bool CRcdLogClient::OnInit(std::string& areaCode, ConfigInfo& mysqlInfo, ConfigInfo& redisInfo)
{
	m_areaCode = areaCode;
	do 
	{
		m_pSqlClient = MakeSqlClient(&mysqlInfo);
		m_pRedisClient = MakeRedisClient(redisInfo.st_address, atoi(redisInfo.st_port.c_str()));
		if(!m_pRedisClient || !m_pSqlClient)
		{
			break;
		}
		else
		{
			return true;
		}
	} while (0);
	
	if (m_pRedisClient)
	{
		delete m_pRedisClient;
		m_pRedisClient = NULL;
	}
	if (m_pSqlClient)
	{
		delete m_pSqlClient;
		m_pSqlClient = NULL;
	}

	return false;
}

void CRcdLogClient::OnKeysInit()
{
	m_keyList.clear();
	std::string key = CUtilRedis::MakeOneKey(m_areaCode, _INSERT_LOG_ESIGN);
	m_keyList.push_back(key);

	key = CUtilRedis::MakeOneKey(m_areaCode, _INSERT_LOG_SIGNVERIFY);
	m_keyList.push_back(key);

	key = CUtilRedis::MakeOneKey(m_areaCode, _INSERT_LOG_SIGNREVOKE);
	m_keyList.push_back(key);

	key = CUtilRedis::MakeOneKey(m_areaCode, _INSERT_LOG_PRODUCEESEAL);
	m_keyList.push_back(key);

	key = CUtilRedis::MakeOneKey(m_areaCode, _INSERT_ESEAL);
	m_keyList.push_back(key);

	key = CUtilRedis::MakeOneKey(m_areaCode, _UPDATE_ESEAL);
	m_keyList.push_back(key);
}

bool CRcdLogClient::OnStart()
{
	OnKeysInit();

	pthread_create(&m_threadid, NULL, ThreadFun, this);
	return true;
}

void CRcdLogClient::OnStop()
{
	m_isStop = true;
	m_pCondition->signalAll();
	pthread_join(m_threadid, NULL);

}


CRedisClient* CRcdLogClient::MakeRedisClient(std::string& address, int port)
{
	CRedisClient* pClient = new CRedisClient();
	if(pClient)
	{
		if(pClient->OnInit(address, port))
			return pClient;
		else
		{
			WLogError("CRcdLogClient::MakeRedisClient::OnInit error!, address = %s, port=%d\n",
				address.c_str(), port);
		}
		delete pClient;
		pClient = NULL;
	}
	return NULL;
}

CSqlClient* CRcdLogClient::MakeSqlClient(ConfigInfo* pInfo)
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
			WLogError("CRcdLogClient::MakeSqlClient::OnInit error!, address = %s, port=%d, user=%s, password=%s, dbname=%s\n",
				pInfo->st_address.c_str(), port, pInfo->st_username.c_str(), pInfo->st_password.c_str(), pInfo->st_dbname.c_str());
		}
		delete pClient;
		pClient = NULL;
	}
	return NULL;
}

int CRcdLogClient::RecordToMysql(std::string& sql)
{
	return m_pSqlClient->ExecSqlCmd(sql.c_str());
}

void CRcdLogClient::RecordFromRedis(std::string& strInKey, std::string& value)
{
	std::string strOnKey = (std::string)"spop " + strInKey.c_str();
	if (m_pRedisClient)
	{
		std::string RedisReply;
		m_pRedisClient->ExecCommand(strOnKey, RedisReply);
		value = Common::Decode((unsigned char *)RedisReply.c_str(), RedisReply.length(), 0);
	}
	else
	{
		WLogError("CRcdLogClient::RecordFromRedis::failed!,strInKey = %s\n", strInKey.c_str());
	}
}

int CRcdLogClient::GetRedisCount(std::string& strInKey)
{
	int reply = 0;
	std::string strOnKey = (std::string)"scard " + strInKey.c_str();
	if (m_pRedisClient)
	{
		m_pRedisClient->ExecCommand(strOnKey, reply);
	}
	else
	{
		WLogError("CRcdLogClient::GetRedisCount::failed!,strInKey = %s\n", strInKey.c_str());
	}
	return reply;
}

void CRcdLogClient::ProccessTargetKey(std::string& sql, std::string& strInKey)
{
	int iret = -1;
	std::string savesql = sql;
	if(strInKey.empty())
		return ;
	if (sql.empty()) //update操作
	{
		std::string Temp;
		RecordFromRedis(strInKey, Temp);
		if (Temp.empty())
		{
			return;
		}
		iret = RecordToMysql(Temp);
		if(iret !=0)
		{
			WLogError("CRcdLogClient::ProccessTargetKey::if-->RecordToMysql::failed!,strInKey = %s, sql = %s\n", 
				strInKey.c_str(), sql.c_str());
		}
	}
	else
	{
		bool isDone = false;
		int redisCount = 0;
		int runCount = 0;
		int iMaxFlag = (int)(m_insert_count/20);
		do 
		{
			int iCount = GetRedisCount(strInKey);

			isDone = (iCount > 0) ? false : true;	//当前key 是否已经处理完成

			int iMax = (iCount > 20) ? 20 : iCount; //每次最多插入 10条数据	

			for (int i = 0; i<iMax; i++)
			{					
				std::string Temp;
				RecordFromRedis(strInKey, Temp);
				if (Temp.length()<1)
				{
					continue;
				}			
				if (redisCount>0)
					sql += ",";
				sql += Temp;
				redisCount += 1;
			}

			if (redisCount>0)
			{
				redisCount = 0;
				time_t starttime, endtime;
				starttime = time(NULL);
				iret = RecordToMysql(sql);
				endtime = time(NULL);
				if(iret !=0)
				{
					WLogError("CRcdLogClient::ProccessTargetKey::else-->RecordToMysql::failed!,strInKey = %s, sql = %s\n", 
						strInKey.c_str(), sql.c_str());
				}
				if (endtime - starttime > 1)
				{
					WLogInfo("CRcdLogClient::ProccessTargetKey::RecordToMysql run time > 1 s, timeindex = %d", endtime-starttime);
				}
				sql = savesql;
				//当插入iMaxFlag * 20条数据后，跳出循环返回，插入其他操作，否则在压力比较大情况下会导致某些日志不能被执行
				if(runCount++ > iMaxFlag) 
					break;
			}

		} while (!isDone);	
	}
}


bool CRcdLogClient::GetSqlHeader(size_t index, std::string& header)
{
	switch (index) //头数据
	{
	case 0:
		header = INSERT_LOG_ESIGN;
		break;
	case 1:
		header = INSERT_LOG_SIGNVERIFY;
		break;
	case 2:
		header = INSERT_LOG_SIGNREVOKE;
		break;
	case 3:
		header = INSERT_LOG_PRODUCEESEAL;
		break;
	case 4:
		header = INSERT_ESEAL;
		break;
	case 5:
		return true;
		break;
	default:
		return false;
	}

	return true;
}

void* CRcdLogClient::ThreadFun(void* arg)
{
	CRcdLogClient* pthisObj = (CRcdLogClient*)arg;
	if (pthisObj)
	{
		pthisObj->OnThreadWorker();
	}
	return NULL;
}

void CRcdLogClient::OnThreadWorker()
{
	int iCount = 0;
	bool isHaveDone = false;
	std::string sqlStr;

	while(!m_isStop)
	{
		sqlStr.clear();
		for (size_t i=0; i<m_keyList.size(); i++)
		{
			//获取redis当前key的值个数  
			iCount = GetRedisCount(m_keyList.at(i));
			if (iCount > 0)
			{
				isHaveDone = false;
				if (GetSqlHeader(i, sqlStr)) //得到sql的头部数据，insert xxxx
				{
					ProccessTargetKey(sqlStr, m_keyList.at(i));
				}
				
			}
			else
			{
				isHaveDone = true;
			}
#ifndef WIN32
			usleep(m_insert_wait);
#else
			Sleep(m_insert_wait);
#endif
		}

		if (isHaveDone)
		{
			m_pCondition->waitForSeconds(WAIT_SECOND);
		}
	}
}

bool CRcdLogClient::OnTestOnline()
{
	if(m_pRedisClient && m_pSqlClient)
	{
		if (m_pRedisClient->TestOnline() == true && 
			m_pSqlClient->TestOnline() == true)
		{
			return true;
		}
	}

	return false;
}
