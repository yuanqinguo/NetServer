#ifndef CEVENT_BASE_MGr_H
#define CEVENT_BASE_MGr_H

#include "event2/event.h"
#include "event2/bufferevent.h" 
#include "locker.h"
#include <cstdlib>
#include <cstring>
#include <vector>
#ifndef WIN32
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

class CEventBaseMgr
{
public:
	CEventBaseMgr(const int iNum, bool enablessl = false);

	~CEventBaseMgr();

	void OnStart();

	void OnStop();

	bool PushTask(evutil_socket_t sockfd);

protected:

	void LibEvInit();

	//管道可读，读回来的值为新建立链接的sockfd
	static void PipeReadCallBack(evutil_socket_t fd, short event, void* arg);
	void OnPipeReadCallback(evutil_socket_t& fd, void* arg);

	//event_base_dispatch阻塞线程
	static void* ThreadFun(void* arg);

	/*event_base回调函数*/
	//读数据回调
	static void SocketReadCallBack(bufferevent* bev, void* arg);
	void OnReadCallback(bufferevent*& bev);

	//写数据回调,可有可无
	static void SocketWirteCallBack(bufferevent* bev, void* arg);
	void OnWirteCallback(bufferevent*& bev);

	//异常回调，此回调将直接关闭连接释放bufferevent
	static void SocketEventCallBack(struct bufferevent *bev, short event, void *arg);
	void OnEventCallback(bufferevent*& bev, short& event);

	//定时器，处理长时间无数据交互的链接
	static void TimeoutCallBack(evutil_socket_t fd, short event, void* arg);
	void OnTimeoutCallback();
private:
	LibEvThread* m_pLibEvThreads;

	int m_iThreadnum;
	bool m_isStop;
	bool m_enablessl;

	CMgrRequest* m_pRequestMgr;
};
#endif //CEVENT_BASE_MGr_H