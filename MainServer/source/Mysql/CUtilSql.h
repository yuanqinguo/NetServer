#ifndef CUTIL_SQL_H
#define CUTIL_SQL_H

#include "CommStruct.h"
#include <string>

class CUtilSql
{
public:
	CUtilSql();
	~CUtilSql();

	static std::string MakeInsertESealSql(ComProtocol& objRequest);

	static std::string MakeInsertLogProduceEsealSql(ComProtocol& objRequest);

	static std::string MakeInsertLogESignSql(ComProtocol& objRequest);

	static std::string MakeInsertLogSignRevokeSql(ComProtocol& objRequest);

	static std::string MakeInsertLogSignVerifySql(ComProtocol& objRequest);

	static std::string MakeUpdateESealSql(ComProtocol& objRequest);

	static std::string MakeSelectESealSql(std::string& key);
protected:
private:
};
#endif