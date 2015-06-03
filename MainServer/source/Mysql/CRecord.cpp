#include "CRecord.h"
#include <string.h>

namespace DataBase
{
	/************************************************************************/
	/* CRecord                                                              */
	/************************************************************************/
	/*
	* 1 单条记录
	* 2 [int ]操作 [""]操作
	*/

	CRecord::CRecord(CField * m_f)
	{
		m_field =m_f;
	}

	CRecord::~CRecord(){};

	void CRecord::SetData(string value)
	{
		m_rs.push_back (value);
	}
	/* [""]操作 */
	string CRecord::operator[](string s)
	{
		if (m_field)
		{

			int iTmp = m_field->GetField_NO(s);
			if (iTmp < 0 ||
				iTmp >= m_rs.size())
			{
				return "";
			}
			return m_rs[iTmp];
		}
		return "";
	}
	string CRecord::operator[](int num)
	{
		if (num < 0 ||
			num >= m_rs.size())
		{
			return "";
		}
		return m_rs[num];
	}
	/* null值判断 */
	bool CRecord::IsNull(int num)
	{
		if (num < 0 ||
			num >= m_rs.size())
		{
			return false;
		}

		if("" == m_rs[num].c_str ())
			return true;
		else
			return false;
	}
	bool CRecord::IsNull(string s)
	{
		if(m_field)
		{
			int iTmp = m_field->GetField_NO(s);
			if (iTmp < 0 ||
				iTmp >= m_rs.size())
			{
				return false;
			}

			if("" == m_rs[iTmp].c_str())
				return true;
			else 
				return false;
		}
		return false;
	}
	/* 主要-功能:用 value tab value 的形式 返回结果 */
	string CRecord::GetTabText()
	{
		string temp;
		for(unsigned int i=0 ;i<m_rs.size();i++)
		{
			temp+=m_rs[i];
			if(i<m_rs.size ()-1)
				temp+="\t";
		}
		return temp;
	}



	/************************************************************************/
	/* CRecordSet                                                           */
	/************************************************************************/
	/*
	* 1 记录集合
	* 2 [int ]操作 [""]操作
	* 3 表结构操作
	* 4 数据的插入修改
	*/
	CRecordSet::CRecordSet()
	{
		res = NULL;
		row = NULL;
		pos = 0;
	}
	CRecordSet::CRecordSet(MYSQL *hSQL)
	{
		res = NULL;
		row = NULL;
		m_Data = hSQL;
		pos = 0;
	}
	CRecordSet::~CRecordSet()
	{
	}
	/*
	* 处理返回多行的查询，返回影响的行数
	* 成功返回行数，失败返回-1
	*/
	int CRecordSet::ExecuteSQL(const char *SQL)
	{
		if ( !mysql_real_query(m_Data,SQL,strlen(SQL)))
		{
			//保存查询结果
			res = mysql_store_result(m_Data );
			//得到记录数量
			m_recordcount = (int)mysql_num_rows(res) ; 
			//得到字段数量
			m_field_num = mysql_num_fields(res) ;
			for (int x = 0 ; fd = mysql_fetch_field(res); x++)
			{
				m_field.m_name.push_back(fd->name);
				m_field.m_type.push_back(fd->type);
			}
			//保存所有数据
			while (row = mysql_fetch_row(res))
			{
				CRecord temp(&m_field);
				for (int k = 0 ; k < m_field_num ; k++ )
				{

					if(row[k]==NULL||(!strlen(row[k])))
					{
						temp.SetData ("");
					}
					else
					{

						temp.SetData(row[k]);
					}

				}
				//添加新记录
				m_s.push_back (temp); 
			}
			mysql_free_result(res ) ;

			return m_s.size();
		}
		return -1;
	}
	/*
	* 向下移动游标
	* 返回移动后的游标位置
	*/
	long CRecordSet::MoveNext()
	{
		return (++pos);
	}
	/* 移动游标 */
	long  CRecordSet::Move(long length)
	{
		int l = pos + length;

		if(l<0)
		{
			pos = 0;
			return 0;
		}else 
		{ 
			if(l >= m_s.size())
			{
				pos = m_s.size()-1;
				return pos;
			}else
			{
				pos = l;
				return pos;
			}
		}

	}
	/* 移动游标到开始位置 */
	bool CRecordSet::MoveFirst()
	{
		pos = 0;
		return true;
	}
	/* 移动游标到结束位置 */
	bool CRecordSet::MoveLast()
	{
		pos = m_s.size()-1;
		return true;
	}
	/* 获取当前游标位置 */
	unsigned long CRecordSet::GetCurrentPos()const
	{
		return pos;
	}
	/* 获取当前游标的对应字段数据 */
	bool CRecordSet::GetCurrentFieldValue(const char * sFieldName,
		char *sValue)
	{
		strcpy(sValue,m_s[pos][sFieldName].c_str());
		return true;
	}

	/* 获取当前游标的对应字段数据 */
	bool CRecordSet::GetCurrentFieldValue(const char * sFieldName,
		std::string& sValue)
	{
		sValue = m_s[pos][sFieldName];
		return true;
	}

	bool CRecordSet::GetCurrentFieldValue(const int iFieldNum,char *sValue)
	{
		strcpy(sValue,m_s[pos][iFieldNum].c_str());
		return true;
	}
	/* 获取游标的对应字段数据 */
	bool CRecordSet::GetFieldValue(long index,const char * sFieldName,
		char *sValue)
	{
		strcpy(sValue,m_s[index][sFieldName].c_str());
		return true;
	}
	bool CRecordSet::GetFieldValue(long index,int iFieldNum,char *sValue)
	{
		strcpy(sValue,m_s[index][iFieldNum].c_str());
		return true;
	}
	/* 是否到达游标尾部 */
	bool CRecordSet::IsEof()
	{ 
		return (pos == m_s.size())?true:false;
	}
	/* 
	* 得到记录数目
	*/
	int CRecordSet::GetRecordCount()
	{
		return m_recordcount;
	}
	/* 
	* 得到字段数目
	*/
	int CRecordSet::GetFieldNum()
	{
		return m_field_num;
	}
	/* 
	* 返回字段
	*/
	CField * CRecordSet::GetField()
	{
		return &m_field;
	}
	/* 返回字段名 */
	const char * CRecordSet::GetFieldName(int iNum)
	{
		if ( !&m_field || iNum < 0 || iNum >= m_field.m_name.size())
		{
			return NULL;
		}
		return m_field.m_name.at(iNum).c_str();
	}
	/* 返回字段类型 */
	const int CRecordSet::GetFieldType(char * sName)
	{	
		if (&m_field)
		{

			int i = m_field.GetField_NO(sName);
			if (i < 0 ||
				i >= m_field.m_type.size())
			{
				return NULL;
			}
			return m_field.m_type.at(i);
		}
		return -1;
	}
	const int CRecordSet::GetFieldType(int iNum)
	{
		if (!&m_field || iNum < 0 ||
			iNum >= m_field.m_type.size())
		{
			return NULL;
		}
		return m_field.m_type.at(iNum);
	}
	/* 
	* 返回指定序号的记录
	*/
	CRecord CRecordSet::operator[](int num)
	{
		if (num < 0 ||
			num >= m_s.size())
		{
			return NULL;
		}
		return m_s[num];
	}

}
