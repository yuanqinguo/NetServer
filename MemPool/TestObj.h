#ifndef TEST_OBJ_H
#define TEST_OBJ_H

#include <iostream>
#include <string>

int g_GlobalIndex = 0;

class TestObj
{
public:
	TestObj()
	{
		m_index = g_GlobalIndex++;
		//std::cout<<"TestObj() Create Obj index = "<<m_index<<std::endl;
	}
	~TestObj()
	{
		//std::cout<<"~TestObj() Delete Obj index = "<<m_index<<std::endl;
	}

	void ForTestPrint()
	{
		std::cout<<"TestObj()::ForTestPrint:: index = "<<m_index<<std::endl;
	}
protected:

private:
	int m_index;
};

#endif//TEST_OBJ_H