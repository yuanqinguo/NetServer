#ifndef CRECORD_H
#define CRECORD_H

#include "CField.h"


namespace DataBase
{


	/*
	* 1 单条记录
	* 2 [int ]操作 [""]操作
	*/
	class CRecord
	{
	public:
		/* 结果集 */
		vector<string> m_rs;
		/* 字段信息 占用4字节的内存 当记录数很大是回产生性能问题 */
		CField *m_field;
	public :
		CRecord(){};
		CRecord(CField* m_f);
		~CRecord();


		void SetData(string value);
		/* [""]操作 */
		string operator[](string s);
		string operator[](int num);
		/* null值判断 */
		bool IsNull(int num);
		bool IsNull(string s);
		/* 用 value tab value 的形式 返回结果 */
		string GetTabText();
	};


	/*
	* 1 记录集合
	* 2 [int ]操作 [""]操作
	* 3 表结构操作
	* 4 数据的插入修改
	*/
	class CRecordSet
	{
	private :
		/* 记录集 */
		vector<CRecord> m_s;
		/* 游标位置*/
		unsigned long pos;
		/* 记录数 */
		int m_recordcount;
		/* 字段数 */
		int m_field_num;
		/* 字段信息 */
		CField  m_field;

		MYSQL_RES * res ;
		MYSQL_FIELD * fd ;
		MYSQL_ROW row;
		MYSQL* m_Data ;
	public :
		CRecordSet();
		CRecordSet(MYSQL *hSQL);
		~CRecordSet();

		/* 处理返回多行的查询，返回影响的行数 */
		int ExecuteSQL(const char *SQL);
		/* 得到记录数目 */
		int GetRecordCount();
		/* 得到字段数目 */
		int GetFieldNum();
		/* 向下移动游标 */
		long MoveNext();
		/* 移动游标 */
		long Move(long length);
		/* 移动游标到开始位置 */
		bool MoveFirst();
		/* 移动游标到结束位置 */
		bool MoveLast();
		/* 获取当前游标位置 */
		unsigned long GetCurrentPos()const;
		/* 获取当前游标的对应字段数据 */
		bool GetCurrentFieldValue(const char * sFieldName,char *sValue);
		bool GetCurrentFieldValue(const int iFieldNum,char *sValue);
		bool GetCurrentFieldValue(const char * sFieldName, std::string& sValue);
		/* 获取游标的对应字段数据 */
		bool GetFieldValue(long index,const char * sFieldName,char *sValue);
		bool GetFieldValue(long index,int iFieldNum,char *sValue);
		/* 是否到达游标尾部 */
		bool IsEof();



		/* 返回字段 */
		CField* GetField();
		/* 返回字段名 */
		const char * GetFieldName(int iNum);
		/* 返回字段类型 */
		const int GetFieldType(char * sName);
		const int GetFieldType(int iNum);
		/* 返回指定序号的记录 */
		CRecord operator[](int num);

	};

}
#endif