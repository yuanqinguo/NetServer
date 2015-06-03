#ifndef locker_h__
#define locker_h__

//信号量，互斥锁和条件变量的类封装
//windows下使用兼容的pthreads-win32
//第三方提供的统一posix线程接口，只做调试用

#include<pthread.h>
#include <semaphore.h>
#ifdef WIN32
#include <time.h>
#else
#include<sys/time.h>
#endif

//信号量
class Sem
{
private:
	sem_t sem_;
public:
	Sem(int value);
	~Sem();
	bool wait();
	bool waitForSeconds(int seconds);
	bool post();
};

//互斥锁
class Locker
{
private:
	pthread_mutex_t mutex_;
public:
	Locker();
	~Locker();
	bool lock();
	bool unlock();
	pthread_mutex_t* GetPthreadMutex();
};

//简单守卫
class LockerGuard
{
private:
	Locker& locker_;
public:
	LockerGuard(Locker& locker);
	~LockerGuard();
};

//条件变量
class Cond
{
private:
	pthread_cond_t cond_;
	pthread_mutex_t mutex_;
public:
	Cond();
	~Cond();
	bool wait();
	bool waitForSeconds(int seconds);
	bool signal();
	bool signalAll();
};

class Condition
{
private:
	pthread_cond_t cond_;
	Locker& mutex;
public:
	Condition(Locker& locker);
	~Condition();
	bool wait();
	bool waitForSeconds(int seconds);
	bool signal();
	bool signalAll();
};
#endif // locker_h__
