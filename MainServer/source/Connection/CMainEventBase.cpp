#include "CMainEventBase.h"
#include "CMgrRequest.h"
#include "wLog.h"

#include <stdio.h>
#include <errno.h>
#define LISTEN_FLAG 1024

CMainEventBase::CMainEventBase()
{
	m_listenerFd = -1;
	m_pMainBase = NULL;
	m_isInited = false;
#ifndef WIN32
	m_pEventBaseMgr = NULL;
#endif
}

CMainEventBase::~CMainEventBase()
{
#ifndef WIN32
	if(m_pEventBaseMgr)
		delete m_pEventBaseMgr;
	m_pEventBaseMgr = NULL;
#endif

}

//初始化
bool CMainEventBase::OnInit(int iPort, int iThNum, bool enablessl)
{
	m_listenerport = iPort;

#ifndef WIN32
	//创建event_base管理对象
	m_pEventBaseMgr = new CEventBaseMgr(iThNum, enablessl);
	if (!m_pEventBaseMgr)
	{
		return false;
	}
#endif

	m_isInited = TcpInit();;

	return m_isInited;
}

typedef struct sockaddr SA;
bool CMainEventBase::TcpInit()
{
	evutil_socket_t listener;
	do{
		listener = ::socket(AF_INET, SOCK_STREAM, 0);
		if( listener == -1 )
			return false;

		//允许多次绑定同一个地址。要用在socket和bind之间
		evutil_make_listen_socket_reuseable(listener);

		struct sockaddr_in sin;
		sin.sin_family = AF_INET;
		sin.sin_addr.s_addr = 0;
		sin.sin_port = htons(m_listenerport);

		if( ::bind(listener, (SA*)&sin, sizeof(sin)) < 0 )
			break;

		if( ::listen(listener, LISTEN_FLAG) < 0)
			break;

#ifdef WIN32
		//跨平台统一接口，将套接字设置为非阻塞状态
		evutil_make_socket_nonblocking(listener);
#endif
		m_listenerFd = listener;

		return true;
	}while(0);

	evutil_closesocket(listener);
	WLogInfo("MainServer::TcpInit!\nstart listener==>port = %d, errno = %d\n", m_listenerport, errno);

	return false;
}

#ifndef WIN32
void CMainEventBase::DoAccept()
{
	struct sockaddr_in chiaddr;
	socklen_t clilen = sizeof( chiaddr );
	evutil_socket_t connfd = -1;
	while(true)
	{
		connfd = -1;
		if( ( connfd  = accept( m_listenerFd, (struct sockaddr *)&chiaddr, &clilen ) ) == -1 )
		{                                        //!> accept 返回的还是套接字
			static int iAcptCount;
			perror("CMainEventBase::accept error! \n");
			WLogError("CMainEventBase::OnAcceptCallback::accept error! iAcptCount = %d\n", iAcptCount++);
			continue;
		}
		m_pEventBaseMgr->PushTask(connfd);
	}
}

void CMainEventBase::OnLinuxStart()
{
	m_pEventBaseMgr->OnStart();
	DoAccept();
}
#else

void CMainEventBase::OnWindowsStart()
{
	m_pMainBase = event_base_new();

	//添加监听客户端请求连接事件
	struct event* ev_listen = event_new(m_pMainBase, m_listenerFd, EV_READ | EV_PERSIST, AcceptCallBack, this);
	event_add(ev_listen, NULL);
	//注册定时器回调
	struct timeval tv;
	tv.tv_sec=10; //间隔10s
	tv.tv_usec=0;
	struct event* evtime = event_new(NULL, -1, 0, NULL, NULL);
	event_assign(evtime, m_pMainBase, -1, EV_TIMEOUT | EV_PERSIST, 
		TimeoutCallBack, this);
	event_add(evtime,&tv);


	WLogInfo("MainServer::OnStart::listenerPort = %d, pthreadID = %d\n", m_listenerport, pthread_self());
	event_base_dispatch(m_pMainBase);

	event_base_free(m_pMainBase);
	m_pMainBase = NULL;
}
#endif

//启动服务
void CMainEventBase::OnStart()
{
	if( m_listenerFd == -1 )
	{
		perror(" tcp_server_init error ");
		exit(-1);
	}	

#ifndef WIN32
	OnLinuxStart();

#else
	OnWindowsStart();

#endif
}

//退出服务
void CMainEventBase::OnStop()
{
	if(!m_isInited)
		return;

	evutil_closesocket(m_listenerFd);

#ifndef WIN32
	m_pEventBaseMgr->OnStop();

#else
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500;
	event_base_loopexit(m_pMainBase, &tv);
#endif
}


#ifdef WIN32
void CMainEventBase::AcceptCallBack(evutil_socket_t fd, short events, void* arg)
{
	CMainEventBase* pThisObj = (CMainEventBase*)arg;
	if(pThisObj)
		pThisObj->OnAcceptCallback(fd, events);
}

void CMainEventBase::OnAcceptCallback(evutil_socket_t& fd, short events)
{
	evutil_socket_t sockfd = -1;
	struct sockaddr_in client;
	int len = sizeof(client);

	sockfd = ::accept(fd, (struct sockaddr*)&client, &len );
	if(sockfd < 0)
	{
		static int iAcptCount;
		perror("CMainEventBase::accept error! \n");
		WLogError("CMainEventBase::OnAcceptCallback::accept error! iAcptCount = %d\n", iAcptCount++);
		return;
	}
	evutil_make_socket_nonblocking(sockfd);

	bufferevent* bev = bufferevent_socket_new(m_pMainBase, sockfd, BEV_OPT_CLOSE_ON_FREE);
	if(!bev)
	{
		WLogError("CMainEventBase::OnAcceptCallback::bufferevent_socket_new error!\n");
		return ;
	}
	bufferevent_setcb(bev, SocketReadCallBack, NULL/*SocketWirteCallBack*/, SocketEventCallBack, this);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);

}

void CMainEventBase::SocketReadCallBack(bufferevent* bev, void* arg)
{
	CMainEventBase* pThisObj = (CMainEventBase*)arg;
	if(pThisObj)
		pThisObj->OnReadCallback(bev);
}

//读数据处理，此函数被调用，说明数据已经从内核区拷贝到bufferevent的缓冲区
void CMainEventBase::OnReadCallback(bufferevent*& bev)
{
	//数据接收,必须确保当前已经把bufferevent中的数据全部读取出来
	char* pMsg = new char[4096];
	size_t len = bufferevent_read(bev, pMsg, 4096);
	pMsg[len] = '\0';

	//请求处理	
	int iRet = -1;
	std::string reply = "Error";

	if (CMgrRequest::GetInstance())
		iRet = CMgrRequest::GetInstance()->HandleRequest(bev, pMsg, reply);

	//若返回数据不完整，表示需要继续接收,先不回复，等待下次回调
	if(iRet != DATA_DEF)
	{
		if(!reply.empty())
			bufferevent_write(bev, reply.c_str(), reply.size());
	}
	delete pMsg;
	pMsg = NULL;
}

void CMainEventBase::SocketWirteCallBack(bufferevent* bev, void* arg)
{
	
}

void CMainEventBase::OnWirteCallback(bufferevent*& bev)
{

}

//异常回调处理
void CMainEventBase::SocketEventCallBack(struct bufferevent *bev, short event, void *arg)
{
	CMainEventBase* pThisObj = (CMainEventBase*)arg;
	if(pThisObj)
		pThisObj->OnEventCallback(bev, event);
}

void CMainEventBase::OnEventCallback(bufferevent*& bev, short& event)
{
	if (event & BEV_EVENT_EOF)
	{
		WLogInfo("connection closed\n");
	}
	else if (event & BEV_EVENT_ERROR)
	{
		WLogError("some other error\n");
	}
	else
	{
		WLogError("unknow error!\n");
	}

	WLogError("CMainEventBase::OnEventCallback::close\n");

	evutil_socket_t sock = bufferevent_getfd(bev);
	//这将自动close套接字和free读写缓冲区
	if (CMgrRequest::GetInstance())
		CMgrRequest::GetInstance()->ReleaseHandler(sock);
	else
		bufferevent_free(bev);
}

void CMainEventBase::TimeoutCallBack(evutil_socket_t fd, short event, void* arg)
{
	CMainEventBase* pThisObj = (CMainEventBase*)arg;
	if (pThisObj)
	{
		pThisObj->OnTimeoutCallback();
	}
}

void CMainEventBase::OnTimeoutCallback()
{
	if (CMgrRequest::GetInstance())
	{
		CMgrRequest::GetInstance()->OnCheckActiveTime();
	}
}
#endif