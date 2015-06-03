#ifndef CSQL_CLIENT_H
#define CSQL_CLIENT_H

#include "locker.h"
#include <mysql.h>
#include <string>

class CSqlClient
{
public:
	CSqlClient();
	
	~CSqlClient();
	
	bool OnInit(std::string& address, unsigned int& port, 
		std::string& user, std::string& password,
		std::string& dbName, unsigned long& flag);

	int ExecSqlCmd(std::string& sql, std::string& result);

	int ExecSqlCmd(const char* sql);

	bool TestOnline();

	bool BeginTrans();

	bool TransRollback();

	bool TransCommit();

protected:
	bool OnConnect();

	bool ReConnect();

	void OnDisConnect();
private:
	unsigned int m_port;
	std::string m_address;
	std::string m_user;
	std::string m_password;
	std::string m_dbName;
	unsigned long m_flag;

	int m_iReconnectCount;

	MYSQL* m_SqlHandle;

	Locker m_locker;
};
#endif