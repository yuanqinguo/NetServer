#include "locker.h"

//信号量

Sem::Sem(int value)
{
	sem_init(&sem_,0,value);
}
Sem::~Sem()
{
	sem_destroy(&sem_);
}
bool Sem::wait()
{
	return sem_wait(&sem_)==0;
}
bool Sem::waitForSeconds(int seconds)
{
	int ret=0;
	struct timespec timeout={0};
#ifdef WIN32
	timeout.tv_sec=time(NULL)+seconds;
#else
	struct  timeval now={0};
	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec+seconds;
#endif
	ret=sem_timedwait(&sem_,&timeout);
	return ret==0;
}
bool Sem::post()
{
	return sem_post(&sem_)==0;
}


//互斥锁

Locker::Locker()
{
	pthread_mutex_init(&mutex_,NULL);
}
Locker::~Locker()
{
	pthread_mutex_destroy(&mutex_);
}
bool Locker::lock()
{
	return pthread_mutex_lock(&mutex_)==0;
}
bool Locker::unlock()
{
	return pthread_mutex_unlock(&mutex_)==0;
}
pthread_mutex_t* Locker::GetPthreadMutex()
{
	return &mutex_;
}


//简单守卫
LockerGuard::LockerGuard(Locker& locker):locker_(locker)
{
	locker_.lock();
}
LockerGuard::~LockerGuard()
{
	locker_.unlock();
}


//条件变量

Cond::Cond()
{
	pthread_mutex_init(&mutex_,NULL);
	pthread_cond_init(&cond_,NULL);
}
Cond::~Cond()
{
	pthread_mutex_destroy(&mutex_);
	pthread_cond_destroy(&cond_);
}
bool Cond::wait()
{
	int ret=0;
	pthread_mutex_lock(&mutex_);
	ret=pthread_cond_wait(&cond_,&mutex_);
	pthread_mutex_unlock(&mutex_);
	return ret==0;
}
bool Cond::waitForSeconds(int seconds)
{
	int ret=0;
	pthread_mutex_lock(&mutex_);
	struct timespec timeout={0};
#ifdef WIN32
	timeout.tv_sec=time(NULL)+seconds;
#else
	struct  timeval now={0};
	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec+seconds;
#endif
	ret=pthread_cond_timedwait(&cond_,&mutex_,&timeout);
	pthread_mutex_unlock(&mutex_);
	return ret==0;
}
bool Cond::signal()
{
	return pthread_cond_signal(&cond_)==0;
}
bool Cond::signalAll()
{
	return pthread_cond_broadcast(&cond_)==0;
}



Condition::Condition(Locker& locker):mutex(locker)
{
	pthread_cond_init(&cond_,NULL);
}
Condition::~Condition()
{
	pthread_cond_destroy(&cond_);
}
bool Condition::wait()
{
	int ret=0;
	ret=pthread_cond_wait(&cond_,mutex.GetPthreadMutex());
	return ret==0;
}
bool Condition::waitForSeconds(int seconds)
{
	int ret=0;
	struct timespec timeout={0};
#ifdef WIN32
	timeout.tv_sec=time(NULL)+seconds;
#else
	struct  timeval now={0};
	gettimeofday(&now,NULL);
	timeout.tv_sec = now.tv_sec+seconds;
#endif
	ret=pthread_cond_timedwait(&cond_,mutex.GetPthreadMutex(),&timeout);
	return ret==0;
}
bool Condition::signal()
{
	return pthread_cond_signal(&cond_)==0;
}
bool Condition::signalAll()
{
	return pthread_cond_broadcast(&cond_)==0;

}