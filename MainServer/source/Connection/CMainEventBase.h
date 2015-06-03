#ifndef CMAIN_SERVER_H
#define CMAIN_SERVER_H

#include "locker.h"

#ifndef WIN32
#include "CEventBaseMgr.h"
#endif

#include "event2/listener.h"
#include "event2/thread.h"
#include "event2/bufferevent.h" 
#include <vector>

class CMainEventBase
{
public:
	CMainEventBase();

	~CMainEventBase();

	//初始化,监听的端口，event_base个数
	bool OnInit(int iPort, int iThNum, bool enablessl = false);

	//启动服务
	void OnStart();

	//退出服务
	void OnStop();

protected:
	bool TcpInit();

	static void AcceptCallBack(evutil_socket_t fd, short events, void* arg);
	void OnAcceptCallback(evutil_socket_t& fd, short events);

#ifndef WIN32
	void OnLinuxStart();
	void DoAccept();
#else
	void OnWindowsStart();
	/*event_base回调函数*/
	//读数据回调
	static void SocketReadCallBack(bufferevent* bev, void* arg);
	void OnReadCallback(bufferevent*& bev);

	//写数据回调
	static void SocketWirteCallBack(bufferevent* bev, void* arg);
	void OnWirteCallback(bufferevent*& bev);

	//异常回调，此回调将直接关闭连接释放bufferevent
	static void SocketEventCallBack(struct bufferevent *bev, short event, void *arg);
	void OnEventCallback(bufferevent*& bev, short& event);

	//定时器，处理长时间无数据交互的链接
	static void TimeoutCallBack(evutil_socket_t fd, short event, void* arg);
	void OnTimeoutCallback();
#endif

private:
	event_base* m_pMainBase; //主线程event_base，只负责accept链接

#ifndef WIN32
	CEventBaseMgr* m_pEventBaseMgr; //工作event_base的管理对象
#endif
	
	evutil_socket_t m_listenerFd; //监听的sock

	int m_listenerport;	//监听端口

	bool m_isInited;	//是否初始化
};


#endif //CMAIN_SERVER_H