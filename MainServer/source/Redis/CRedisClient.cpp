#include "CRedisClient.h"
#include "wLog.h"

CRedisClient::CRedisClient()
{
	m_pRedisContext = NULL;
	m_locker = new Locker();
}

CRedisClient::~CRedisClient()
{
	{
		LockerGuard guard(*m_locker);
		if( m_pRedisContext )
		{
			redisFree(m_pRedisContext);
			m_pRedisContext = NULL;
		}
	}
	if(m_locker)
		delete m_locker;
	m_locker  = NULL;
}

bool CRedisClient::OnInit(std::string& address, int& port)
{
	m_address = address;
	m_port = port;
	struct timeval timeout = { 1, 500000 }; // 1.5 seconds 首次链接
	return OnConnect(&timeout);
}

bool CRedisClient::TestOnline()
{
	m_locker->lock();
	std::string cmd = "PING";
	redisReply* pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );
	if (pReply != NULL)
	{
		m_locker->unlock();
		freeReplyObject(pReply);
		pReply = NULL;
		return true;
	}
	else
	{//链接已经断开，开始重连计数，重连三次以后链接不上则认为此数据库已下线，将在client管理中进行剔除
		m_locker->unlock();
		if(ReConnect())
		{
			WLogInfo("CRedisClient::TestOnline::RedisClient Re Online! ip=%s, port=%d\n", m_address.c_str(), m_port);
			m_iReconnectCount = 0;
			return true;
		}
		else
		{
			m_iReconnectCount++;
			if (m_iReconnectCount > 3)
			{
				WLogError("CRedisClient::TestOnline::RedisClient Off! ip=%s, port=%d\n", m_address.c_str(), m_port);
				return false;
			}
			else
			{
				return true;
			}
		}
	}

	return false;
}

bool CRedisClient::OnConnect(struct timeval* tvl)
{
	m_pRedisContext = redisConnectWithTimeout(m_address.c_str(), m_port, *tvl);
	if ( m_pRedisContext == NULL || m_pRedisContext->err)
	{
		if (m_pRedisContext)
		{
			WLogError("Connection error: %s\n", m_pRedisContext->errstr);
			redisFree(m_pRedisContext);
			m_pRedisContext = NULL;
		}
		else
		{
			WLogError("Connection error: can't allocate redis context\n");
		}
		return false;
	}

	WLogInfo("CRedisClient::OnConnected success!\n");
	return true;
}

bool CRedisClient::ReConnect()
{
	LockerGuard guard(*m_locker);
	if( m_pRedisContext )
	{
		redisFree(m_pRedisContext);
		m_pRedisContext = NULL;
	}
	struct timeval timeout = { 0, 500000 }; // 0.5 seconds 重连
	return OnConnect(&timeout);
}

int CRedisClient::ExecCommand(std::string& cmd)
{
	LockerGuard guard(*m_locker);
	int iRet = -1;
	redisReply* pReply = NULL;
	pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );
	if(pReply)
	{
		freeReplyObject(pReply);
		return 0;
	}
	return -1;
}

int CRedisClient::ExecCommand(std::string& cmd, std::string& result)
{
	LockerGuard guard(*m_locker);
	int iRet = -1;
	redisReply* pReply = NULL;
	pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );

	if (pReply)
	{
	
		if(pReply->type == REDIS_REPLY_STRING)
		{
			if(pReply->str)
			{
				result = pReply->str;
				iRet = 0;
			}
		}
		freeReplyObject(pReply);
		pReply = NULL;
	}
	return iRet;
}

int CRedisClient::ExecCommand(std::string& cmd, int& result)
{
	LockerGuard guard(*m_locker);
	int iRet = -1;
	redisReply* pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );
	if (pReply)
	{

		if(pReply->type == REDIS_REPLY_INTEGER)
		{
			result = (int)pReply->integer;
			iRet = 0;
		}
		freeReplyObject(pReply);
		pReply = NULL;
	}

	return iRet;

}

int CRedisClient::ExecCommand(std::string& cmd, std::vector<int>& result)
{
	result.clear();
	LockerGuard guard(*m_locker);
	int iRet = -1;
	redisReply* pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );
	if (pReply)
	{

		if(pReply->type == REDIS_REPLY_ARRAY)
		{
			int itmp = -1;
			for (size_t i=0; i<pReply->elements; i++)
			{
				redisReply* pTmp = pReply->element[i];
				if(pTmp)
				{
					if (pTmp->type == REDIS_REPLY_INTEGER)
					{
						int iTmp = (int)pTmp->integer;
						result.push_back(iTmp);
					}
				}
			}
			iRet = 0;
		}
		freeReplyObject(pReply);
		pReply = NULL;
	}

	return iRet;
}

int CRedisClient::ExecCommand(std::string& cmd, std::vector<std::string>& result)
{
	LockerGuard guard(*m_locker);
	int iRet = -1;
	redisReply* pReply = (redisReply*)redisCommand( m_pRedisContext ,cmd.c_str() );
	if (pReply)
	{

		if(pReply->type == REDIS_REPLY_ARRAY)
		{
			int itmp = -1;
			redisReply* pTmp = NULL;
			for (size_t i=0; i<pReply->elements; i++)
			{
				redisReply* pTmp = pReply->element[i];
				if(pTmp)
				{
					if (pTmp->type == REDIS_REPLY_STRING)
					{
						if (pTmp->str)
						{
							iRet = 0;
							std::string Tmp = pTmp->str;
							result.push_back(Tmp);
						}
					}
				}
				pTmp = NULL;
			}
		}

		freeReplyObject(pReply);
		pReply = NULL;
	}

	return iRet;
}

int CRedisClient::ExecCommand(const char* cmd, std::string& result)
{
	std::string req = cmd;
	return ExecCommand(req, result);
}

int CRedisClient::ExecCommand(const char* cmd, int& result)
{
	std::string req = cmd;
	return ExecCommand(req, result);
}

int CRedisClient::ExecCommand(const char* cmd, std::vector<int>& result)
{
	std::string req = cmd;
	return ExecCommand(req, result);
}

int CRedisClient::ExecCommand(const char* cmd, std::vector<std::string>& result)
{
	std::string req = cmd;
	return ExecCommand(req, result);
}

int CRedisClient::ExecCommand(const char* cmd)
{
	std::string req = cmd;
	return ExecCommand(req);
}