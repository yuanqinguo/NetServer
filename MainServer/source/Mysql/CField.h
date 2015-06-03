#ifndef CFIELD_H
#define CFIELD_H

#include <mysql.h>
#include <iostream>
#include <vector>
#include <string>

using namespace std;

namespace DataBase
{

	/*
	* 字段操作 
	*/
	class CField
	{
	public :
		/* 字段名称 */
		vector<string> m_name;
		/* 字段类型 */
		vector<enum_field_types> m_type;
	public :
		CField();
		~CField();


		/* 是否是数字 */
		bool IsNum(int num);
		/* 是否是数字 */
		bool IsNum(string num);
		/* 是否是日期 */
		bool IsDate(int num);
		/* 是否是日期 */
		bool IsDate(string num);
		/* 是否是字符 */
		bool IsChar(int num);
		/* 是否是字符 */
		bool IsChar(string num);
		/* 是否为二进制数据 */
		bool IsBlob(int num);
		/* 是否为二进制数据 */
		bool IsBlob(string num);
		/* 得到指定字段的序号 */
		int GetField_NO(string field_name);
	};

}
#endif//CFIELD_H