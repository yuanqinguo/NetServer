#include "MemChunk.h"
#include "TestObj.h"
#include "TestPool.h"
#include <time.h>

time_t GetCurrentStringTime(std::string& timeStr, int iIndex)
{
	std::string tmpStr;
	char buf[32] = {0};

#ifdef _WIN32
	time_t t;
	tm *tp;
	t = time(NULL);
	tp = localtime(&t);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", 
		tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday
		,tp->tm_hour, tp->tm_min, 
		(tp->tm_sec - iIndex > 0) ? tp->tm_sec - iIndex: 0);

	timeStr = buf;
	return t - iIndex;
#else
	time_t now;
	time(&now);
	now = now - iIndex;
	struct tm timenow;
	localtime_r(&now, &timenow);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timenow);
	timeStr = buf;
	return now;
#endif

}

#define MAX_NEW 100000000
int main(int argc, char** argv)
{
	std::string start, end;
	ObjectManager<TestObj> obj;
	MemoryPool pool;
	int i=0;
	GetCurrentStringTime(start, 0);
	while(i++<MAX_NEW)
	{
		TestObj* pObj = obj.Create(&pool);
		obj.Delete(&pool, pObj);
	}
	GetCurrentStringTime(end, 0);

	std::cout<<"Pool::start="<<start.c_str()<<" end="<<end.c_str()<<std::endl;
	getchar();

	GetCurrentStringTime(start, 0);
	i = 0;
	while(i++<MAX_NEW)
	{
		TestObj* pObj = new TestObj;
		delete pObj;
	}
	GetCurrentStringTime(end, 0);
	std::cout<<"While new ::start="<<start.c_str()<<" end="<<end.c_str()<<std::endl;
	getchar();

	return 0;
}