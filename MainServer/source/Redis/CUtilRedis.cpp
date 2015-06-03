#include "CUtilRedis.h"

CUtilRedis::CUtilRedis()
{

}

CUtilRedis::~CUtilRedis()
{

}

std::string CUtilRedis::MakeGetRedisCmd(std::string& key)
{
	std::string reply;
	reply = "GET " + key;

	return reply;
}

std::string CUtilRedis::MakeSetRedisCmd(std::string& key, std::string& value)
{
	std::string reply;
	reply = "SET " + key + " " + value;
	
	return reply;
}

std::string CUtilRedis::MakeOneKey(std::string& header, const char* arg)
{
	std::string key;
	key = header.substr(0,4) + arg;
	return  key;
}

std::string CUtilRedis::MakeSAddRedisCmd(std::string& key, std::string& value)
{
	std::string reply;
	reply = "SADD " + key + " " + value;

	return reply;
}