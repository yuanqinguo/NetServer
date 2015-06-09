#ifndef CMAIN_SERVER_H
#define CMAIN_SERVER_H

#include "CEventBaseMgr.h"
#include "openssl/ssl.h"
#include "locker.h"

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
	SSL* CreateSSL(evutil_socket_t& fd);
	//事件回调
	static void EventCallBack(evutil_socket_t fd, short event, void* arg);

	static int SslRecv(SSL* ssl, char* buffer, int ilen);
	static int NormalRecv(evutil_socket_t& fd, char* buffer, int ilen);

	static int SslSend(SSL* ssl, const char* buffer, int ilen);
	static int NormalSend(evutil_socket_t& fd, const char* buffer, int ilen);

	//定时器，处理长时间无数据交互的链接
	static void TimeoutCallBack(evutil_socket_t fd, short event, void* arg);
	void OnTimeoutCallback();
#endif

private:
	event_base* m_pMainBase;

#ifndef WIN32
	CEventBaseMgr* m_pEventBaseMgr; //工作event_base的管理对象
#endif
	
	evutil_socket_t m_listenerFd; //监听的sock

	int m_listenerport;	//监听端口

	bool m_isInited;	//是否初始化
};
#endif //CMAIN_SERVER_H