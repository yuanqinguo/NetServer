#ifndef CEVENT_BASE_MGr_H
#define CEVENT_BASE_MGr_H

#include "event2/event.h"
#include "openssl/ssl.h"
#include "locker.h"

#include <cstdlib>
#include <cstring>
#include <vector>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif



/*
* event_base 对象管理
*/

class CMgrRequest;

typedef struct  
{
	event_base* pBase;
	void* pParam;
	struct event* pNotifyEvt;
	pthread_t pid;
	int recvfd;
	int sendfd;
}LibEvThread;

typedef struct event_ctx
{
	struct event* pEvent;
	SSL* pSsl;
	evutil_socket_t sock;

	event_ctx()
	{
		pEvent = NULL;
		pSsl = NULL;
		sock = -1;
	}

	~event_ctx()
	{
		if (pEvent)
		{
			event_del(pEvent);
			event_free(pEvent);
			pEvent = NULL;
		}
		if(pSsl)
		{
			if(pSsl->ctx)
				SSL_CTX_free(pSsl->ctx);
			SSL_free(pSsl);	
		}
		evutil_closesocket(sock);
		sock = -1;
	}
}EventCtx;

#define EventCtx_free(x) 	\
	if (x) \
	{ delete x;	x = NULL; }

class CEventBaseMgr
{
public:
	CEventBaseMgr(const int iNum);

	~CEventBaseMgr();

	void OnStart();

	void OnStop();

	bool PushTask(evutil_socket_t sockfd);

protected:

	void LibEvInit();
	SSL* CreateSSL(evutil_socket_t& sockfd);

	static int SslRecv(SSL* ssl, char* buffer, int ilen);
	static int NormalRecv(evutil_socket_t& fd, char* buffer, int ilen);

	static int SslSend(SSL* ssl, const char* buffer, int ilen);
	static int NormalSend(evutil_socket_t& fd, const char* buffer, int ilen);

	//管道可读，读回来的值为新建立链接的sockfd
	static void PipeReadCallBack(evutil_socket_t fd, short event, void* arg);
	void OnPipeReadCallback(evutil_socket_t& fd, void* arg);

	//event_base_dispatch阻塞线程
	static void* ThreadFun(void* arg);

	//回调
	static void EventCallBack(evutil_socket_t fd, short event, void* arg);

	//定时器，处理长时间无数据交互的链接
	static void TimeoutCallBack(evutil_socket_t fd, short event, void* arg);
	void OnTimeoutCallback();
private:
	LibEvThread* m_pLibEvThreads;

	int m_iThreadnum;
	bool m_isStop;

	CMgrRequest* m_pRequestMgr;
};
#endif //CEVENT_BASE_MGr_H