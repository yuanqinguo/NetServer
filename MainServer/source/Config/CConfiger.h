#ifndef CCONFIG_H
#define CCONFIG_H

#include "locker.h"
#include "CommStruct.h"
#include <map>
#include <vector>
#include <stdlib.h>

/*
* 配置对象，读取配置文件，为全局提供配置获取服务
* 
* 配置文件格式：
* HuNan=mysql&431027&192.168.1.211&3306&admin&123456
* 湖南区=服务器类型&区号&ip地址&端口&用户名&密码
* GuangXi=redis&431028&192.168.1.210&3306&&admin&123456
* 广西区=服务器类型&区号&ip地址&端口&用户名&密码
* threads=10	服务器业务线程数
* insert_wait=100 插入等待时间 毫秒
* insert_max_count=100 每次插入记录的最大条数
* enable_record=true or false 当前节点是否启用印章操作记录模块
*/
class CConfiger
{
private:
	CConfiger();

public:
	~CConfiger();

	static CConfiger* GetInstance();
	static void DelInstance();

	bool OnInit(const std::string& filename);

	void GetConfigInfo(int iZoneCode, ConfigInfo& info, bool isRedis = false);
	void GetConfigInfos(std::map<int, ConfigInfo>& configs, bool isRedis = false);

	int GetCfgThreads()
	{
		return m_threads;
	}
	bool GetEnableRecord()
	{
		return m_isEnableRecord;
	}
	int GetInsertWait()
	{
		return m_insert_wait;
	}
	int GetInsertCount()
	{
		return m_insert_count;
	}

	bool GetEnableSSL()
	{
		return m_isEnablessl;
	}
protected:
	bool ReadLine(std::ifstream& ifs, std::string& outString);

	bool ParaseLine(const std::string& str);

	//解析服务器性能配置项，如线程数，每次插入记录条数，模块是否启用等
	bool PerformanceItem(const std::string& str);

	//解析数据源配置项，如mysql  redis相关
	bool SourceDataItme(const std::string& str);

	void SplitString(const std::string& str, std::vector<std::string>& list);
	void SplitString2(std::string s, char splitchar, std::vector<std::string>& list);

private:
	static Locker m_locker;
	static CConfiger* m_pInstance;

	std::string m_fileName;

	Locker m_configMysqlLocker;
	std::map<int, ConfigInfo*> m_configMysqlMap;

	Locker m_configRedisLocker;
	std::map<int, ConfigInfo*> m_configRedisMap;

	int m_threads;
	bool m_isEnableRecord;
	bool m_isEnablessl;
	int m_insert_wait;
	int m_insert_count;
	int m_iLine;
};

#endif