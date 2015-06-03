#include "CConfiger.h"
#include "wLog.h"
#include <fstream>

#define TYPE_ANNO "#"							//注释行
#define TYPE_REDIS "redis"						//redis配置行
#define TYPE_MYSQL "mysql"						//mysql配置行
#define TYPE_THREADS "threads"					//业务线程配置行
#define TYPE_INSERT_WAIT "insert_wait"			//插入等待时间 毫秒
#define TYPE_INSERT_COUNT "insert_max_count"	//每次插入记录的最大条数
#define TYPE_ENABLE_RECORD "enable_record"		//当前节点是否启用印章操作记录模块

Locker CConfiger::m_locker;
CConfiger* CConfiger::m_pInstance = NULL;

CConfiger::CConfiger()
{
	m_configMysqlMap.clear();
	m_configRedisMap.clear();
}

CConfiger::~CConfiger()
{
	{	
		LockerGuard guard(m_configMysqlLocker);
		std::map<int, ConfigInfo*>::iterator it;
		for (it=m_configMysqlMap.begin(); it != m_configMysqlMap.end(); it++)
		{
			ConfigInfo* pTmp = it->second;
			delete pTmp;
			pTmp = NULL;
		}
		m_configMysqlMap.clear();
	}
	{
		LockerGuard guard(m_configRedisLocker);
		std::map<int, ConfigInfo*>::iterator it;
		for (it=m_configRedisMap.begin(); it != m_configRedisMap.end(); it++)
		{
			ConfigInfo* pTmp = it->second;
			delete pTmp;
			pTmp = NULL;
		}
		m_configRedisMap.clear();
	}
}

CConfiger* CConfiger::GetInstance()
{
	if (NULL == m_pInstance)
	{
		LockerGuard guard(m_locker);
		if (NULL == m_pInstance)
		{
			m_pInstance = new CConfiger();
		}
	}

	return m_pInstance;
}

void CConfiger::DelInstance()
{
	if(m_pInstance != NULL)
		delete m_pInstance;

	m_pInstance = NULL;
}

bool CConfiger::OnInit(const std::string& filename)
{
	int iCount = 0;
	m_fileName.clear();
	m_fileName = filename;
	std::ifstream ifs;
	ifs.open (m_fileName.c_str(), std::ifstream::in);
	if (ifs.is_open())
	{
		bool isDone = false; 
		do 
		{
			std::string outStr;
			isDone = ReadLine(ifs, outStr);//读取一行
			if(outStr.empty() && isDone && iCount < 4)//计数空行，最多允许四行空行
			{
				isDone = false;
				iCount++;
				WLogInfo("CConfiger::OnInit::ReadLine=>%s, iCount %d\n", outStr.c_str(), iCount);
				if(iCount > 4)
					break;
				continue;
			}

			WLogInfo("CConfiger::OnInit::ReadLine=>%s\n", outStr.c_str());
			if(!isDone && outStr[0] != '\r')
			{
				iCount = 0;
				if (ParaseLine(outStr) != true) //解析错误，说明文件格式不对
				{
					WLogError("CConfiger::OnInit::Config file error in line:%d\n", m_iLine);
					return false;
				}
			}
		} while (!isDone);

		return true;
	}

	WLogError("CConfiger::OnInit::Config file error in line:%d\n", m_iLine);
	return false;
}

void CConfiger::GetConfigInfo(int iZoneCode, ConfigInfo& info, bool isRedis)
{
	if(isRedis)
	{
		LockerGuard guard(m_configRedisLocker);
		std::map<int, ConfigInfo*>::iterator it;
		it = m_configRedisMap.find(iZoneCode);
		if(it != m_configRedisMap.end())
		{
			info.st_address = it->second->st_address;
			info.st_password = it->second->st_password;
			info.st_port = it->second->st_port;
			info.st_username = it->second->st_username;
		}
	}
	else
	{
		LockerGuard guard(m_configMysqlLocker);
		std::map<int, ConfigInfo*>::iterator it;
		it = m_configMysqlMap.find(iZoneCode);
		if(it != m_configMysqlMap.end())
		{
			info.st_address = it->second->st_address;
			info.st_password = it->second->st_password;
			info.st_port = it->second->st_port;
			info.st_username = it->second->st_username;
			info.st_dbname = it->second->st_dbname;
		}
	}
}

void CConfiger::GetConfigInfos(std::map<int, ConfigInfo>& configs, bool isRedis)
{
	configs.clear();
	if(isRedis)
	{
		LockerGuard guard(m_configRedisLocker);
		std::map<int, ConfigInfo*>::iterator it;
		for (it=m_configRedisMap.begin(); it != m_configRedisMap.end(); it++)
		{
			ConfigInfo info;
			info.st_address = it->second->st_address;
			info.st_password = it->second->st_password;
			info.st_port = it->second->st_port;
			info.st_username = it->second->st_username;

			configs.insert(std::make_pair(it->first, info));
		}
	}
	else
	{
		LockerGuard guard(m_configMysqlLocker);
		std::map<int, ConfigInfo*>::iterator it;
		for (it=m_configMysqlMap.begin(); it != m_configMysqlMap.end(); it++)
		{
			ConfigInfo info;
			info.st_address = it->second->st_address;
			info.st_password = it->second->st_password;
			info.st_port = it->second->st_port;
			info.st_username = it->second->st_username;
			info.st_dbname = it->second->st_dbname;

			configs.insert(std::make_pair(it->first, info));
		}
	}
	
}

bool CConfiger::ReadLine(std::ifstream& ifs, std::string& outString)
{
	if(ifs.is_open())
	{
		char line[128] = {0};
		ifs.getline(line, 128);
		outString = line;
		m_iLine = m_iLine + 1;
		if (outString.empty())
		{
			return true; //文件已读完
		}
		if(!outString.compare("\r"))
		{
			outString = "";
			return true;
		}

		if(outString.at(outString.size() -1) == '\r')
		{
			outString = outString.substr(0, outString.size() -1);
		}
		return false; //文件可能未读完
	}
	return true;
}

bool CConfiger::ParaseLine(const std::string& str)
{
	if(PerformanceItem(str))
		return true;

	return SourceDataItme(str);
}

bool CConfiger::PerformanceItem(const std::string& str)
{
	std::size_t pos = str.find("=");
	if (pos != std::string::npos)
	{
		std::string header = str.substr(0, pos);
		if (!header.compare(TYPE_THREADS))
		{
			m_threads = atoi(str.substr(pos+1).c_str());
			return true;
		}
		if (!header.compare(TYPE_INSERT_COUNT))
		{
			m_insert_count = atoi(str.substr(pos+1).c_str());
			return true;
		}

		if (!header.compare(TYPE_INSERT_WAIT))
		{
			m_insert_wait = atoi(str.substr(pos+1).c_str());
			return true;
		}

		if (!header.compare(TYPE_ENABLE_RECORD))
		{
			m_isEnableRecord = ( ( str.substr(pos+1).compare("true")==0 ) ? (true) : (false));
			return true;
		}
	}

	return false;
}

bool CConfiger::SourceDataItme(const std::string& str)
{
	std::vector<std::string> list;
	SplitString(str, list);

	if(list.size() > 0)
	{
		if (!list.at(0).compare(TYPE_ANNO))
		{
			return true;
		}
		else if (!list.at(0).compare(TYPE_MYSQL)) //mysql 配置
		{
			if (list.size() < 7)
			{
				return false;
			}
			LockerGuard guard(m_configMysqlLocker);
			ConfigInfo* pInfo = new ConfigInfo();
			int iZoneCode = atoi(list.at(1).c_str());
			pInfo->st_address = list.at(2);
			pInfo->st_port = list.at(3);
			pInfo->st_username = list.at(4);
			pInfo->st_password = list.at(5);
			pInfo->st_dbname = list.at(6);

			m_configMysqlMap.insert(std::make_pair(iZoneCode, pInfo));
		}
		else if (!list.at(0).compare(TYPE_REDIS)) //redis 配置
		{
			LockerGuard guard(m_configRedisLocker);
			ConfigInfo* pInfo = new ConfigInfo();
			int iZoneCode = -1;
			if(list.size() > 1)
				iZoneCode = atoi(list.at(1).c_str());
			if(list.size() > 2)
				pInfo->st_address = list.at(2);
			if(list.size() > 3)
				pInfo->st_port = list.at(3);
			if(list.size() > 4)
				pInfo->st_username = list.at(4);
			if(list.size() > 5)
				pInfo->st_password = list.at(5);

			m_configRedisMap.insert(std::make_pair(iZoneCode, pInfo));
		}
		else //未知，返回错误
		{
			return false;
		}

		return true;
	}
	return false;
}

void CConfiger::SplitString(const std::string& str, std::vector<std::string>& list)
{
	if (!str.empty() && str.at(0) == '#')
	{
		list.push_back("#");
		return;
	}
	std::size_t pos = str.find("=");
	if (pos != std::string::npos)
	{
		std::string tmpstr = str.substr(pos+1);
		SplitString2(tmpstr, '&', list);
	}
}

void CConfiger::SplitString2(std::string s, char splitchar, std::vector<std::string>& vec)
{
	if(vec.size()>0)//保证vec是空的
		vec.clear();
	int length = s.length();
	int start=0;
	for(int i=0;i<length;i++)
	{
		if(s[i] == splitchar && i == 0)//第一个就遇到分割符
		{
			start += 1;
		}
		else if(s[i] == splitchar)
		{
			vec.push_back(s.substr(start,i - start));
			start = i+1;
		}
		else if(i == length-1)//到达尾部
		{
			vec.push_back(s.substr(start,i+1 - start));
		}
	}
}