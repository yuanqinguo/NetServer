#ifndef CUTIL_REDIS_H
#define CUTIL_REDIS_H

#include "CommStruct.h"
#include <string>

class CUtilRedis
{
public:
	CUtilRedis();
	~CUtilRedis();

	static std::string MakeOneKey(std::string& header, const char* arg);
	
	static std::string MakeGetRedisCmd(std::string& key);

	static std::string MakeSetRedisCmd(std::string& key, std::string& value);

	static std::string MakeSAddRedisCmd(std::string& key, std::string& value);
protected:
private:
};
#endif