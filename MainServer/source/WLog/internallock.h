#ifndef __INTERNALLOCK_H__
#define __INTERNALLOCK_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef WIN32
#include <windows.h>
#else
#include <pthread.h>
#endif

class CResGuard
{
public:
	CResGuard()
	{
#ifdef WIN32
		InitializeCriticalSection(&m_lock);
#else
		pthread_mutex_init(&m_lock, NULL);
#endif
	}
	~CResGuard()
	{
#ifdef WIN32
		DeleteCriticalSection(&m_lock);
#else
		pthread_mutex_destroy(&m_lock);
#endif
	}

public:
	class Defend
	{
		public:
			Defend(CResGuard &rg):m_rg(rg)
			{
#ifdef WIN32
				EnterCriticalSection(&m_rg.m_lock);
#else
				pthread_mutex_lock(&m_rg.m_lock);
#endif
			}
			~Defend()
			{
#ifdef WIN32
				LeaveCriticalSection(&m_rg.m_lock);
#else
				pthread_mutex_unlock(&m_rg.m_lock);
#endif
			}
		private:
			CResGuard	&m_rg;
	};

private:
#ifdef WIN32
	CRITICAL_SECTION	m_lock;
#else
	pthread_mutex_t     m_lock;
#endif
};

#endif
