#ifndef CREDIS_CLIENT_H
#define CREDIS_CLIENT_H

#include <hiredis.h>
#include <string>
#include <stdlib.h>
#include <vector>
#include "locker.h"

/*
* RedisClient 对象
*/

class CRedisClient
{
public:

	CRedisClient();

	~CRedisClient();

	bool OnInit(std::string& address, int& port);

	int ExecCommand(std::string& cmd, std::string& result);
	int ExecCommand(std::string& cmd, int& result);
	int ExecCommand(std::string& cmd, std::vector<int>& result);
	int ExecCommand(std::string& cmd, std::vector<std::string>& result);
	int ExecCommand(std::string& cmd);

	int ExecCommand(const char* cmd, std::string& result);
	int ExecCommand(const char* cmd, int& result);
	int ExecCommand(const char* cmd, std::vector<int>& result);
	int ExecCommand(const char* cmd, std::vector<std::string>& result);
	int ExecCommand(const char* cmd);

	//测试链接是否可用，若不可用，则将进行重连
	bool TestOnline();

protected:

	bool ReConnect();

	bool OnConnect(struct timeval* tvl);

private:
	redisContext* m_pRedisContext;
	Locker* m_locker;

	std::string m_address;
	int m_port;
	int m_iReconnectCount;
};
#endif