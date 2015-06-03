#include "CEventBaseMgr.h"
#include "wLog.h"
#include "CMgrRequest.h"


#include <cstdlib>
#ifdef WIN32
#include <io.h>
#else
#include "event2/bufferevent_ssl.h"
#include "openssl/ssl.h"
#endif

CEventBaseMgr::CEventBaseMgr(const int iNum, bool enablessl)
{
	m_iThreadnum = iNum;
	m_isStop = false;
	m_enablessl = enablessl;
	m_pLibEvThreads = NULL;
	m_pRequestMgr = CMgrRequest::GetInstance();

	LibEvInit();

#ifndef WIN32
	if(m_enablessl)
	{
		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();
	}
#endif
}

CEventBaseMgr::~CEventBaseMgr()
{

}

void CEventBaseMgr::LibEvInit()
{
	m_pLibEvThreads = new LibEvThread[m_iThreadnum];
	if(!m_pLibEvThreads)
	{
		WLogError("CEventBaseMgr::LibEvInit::New Error! \n");
		exit(-1);
	}

	for (int i = 0; i < m_iThreadnum; i++) 
	{
		int fds[2];
#ifndef WIN32
		if (pipe(fds)) 
		{
			perror("Can't create notify pipe");
			exit(1);
		}
		//evutil_make_socket_blocking(fds[0]);
		evutil_make_socket_nonblocking(fds[1]);
#endif

		event_base* pBase = event_base_new();
		if (!pBase)
		{
			WLogError("CEventBaseMgr::OnStart::New Error! \n");
			exit(-1);
		}
		m_pLibEvThreads[i].pParam = this;
		m_pLibEvThreads[i].pBase = pBase;
		m_pLibEvThreads[i].recvfd = fds[0];
		m_pLibEvThreads[i].sendfd = fds[1];
	}
}

void CEventBaseMgr::OnStart()
{
	WLogInfo("CEventBaseMgr::OnStart!!\n");
	struct timeval tv;
	tv.tv_sec=10; //间隔10s
	tv.tv_usec=0;

	for(int i=0; i<m_iThreadnum; i++)
	{
		event_base* pBase = m_pLibEvThreads[i].pBase;
		int pipefd = m_pLibEvThreads[i].recvfd;

		//注册pipe读回调
		struct event* ev = event_new(NULL, -1, 0, NULL, NULL);
		event_assign(ev, pBase, pipefd, EV_READ | EV_PERSIST,
			PipeReadCallBack, (void*)&m_pLibEvThreads[i]);
		event_add(ev, NULL);

		//注册定时器回调
		struct event* evtime = event_new(NULL, -1, 0, NULL, NULL);
		event_assign(evtime, pBase, -1, EV_TIMEOUT | EV_PERSIST, 
			TimeoutCallBack, this);
		event_add(evtime,&tv);

		pthread_create(&m_pLibEvThreads[i].pid, NULL, ThreadFun, (void*)&m_pLibEvThreads[i]);
	}
}

void CEventBaseMgr::OnStop()
{
	m_isStop = true;
	struct timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 500;

	for(int i=0; i<m_iThreadnum; i++)
	{
		event_base_loopexit(m_pLibEvThreads[i].pBase, &tv);
		pthread_join(m_pLibEvThreads[i].pid, NULL);
		event_free(m_pLibEvThreads[i].pNotifyEvt);

		evutil_closesocket(m_pLibEvThreads[i].recvfd);
		evutil_closesocket(m_pLibEvThreads[i].sendfd);
	}

	delete []m_pLibEvThreads;
	m_pLibEvThreads = NULL;
}

bool CEventBaseMgr::PushTask(evutil_socket_t sockfd)
{
	if(sockfd < 0)
	{
		WLogError("CEventBaseMgr::PushTask::socketfd < 0\n");
		return false;
	}

	int iflag = m_iThreadnum;
	int index = (sockfd % iflag);
	LibEvThread* pLibEvThread = &m_pLibEvThreads[index];

	int len = write(pLibEvThread->sendfd, (char*)&sockfd, sizeof(evutil_socket_t));

	if ( len != sizeof(evutil_socket_t))
	{
		WLogInfo("Writing to thread notify pipe");
		return false;
	}

	return true;
}

//event_base线程函数
void* CEventBaseMgr::ThreadFun(void* arg)
{
	static int iThread;
	LibEvThread* pLibEvThread = (LibEvThread*)arg;
	WLogInfo("CEventBaseMgr::ThreadFun::event_base_loop! Thread index = %d\n", iThread++);
	event_base_loop(pLibEvThread->pBase, 0);
	return NULL;
}



void CEventBaseMgr::PipeReadCallBack(evutil_socket_t fd, short event, void* arg)
{
	LibEvThread* pLibEvThread = (LibEvThread*)arg;
	if(!pLibEvThread)
		return;

	event_base* pBase = pLibEvThread->pBase;
	evutil_socket_t sockfd = -1;
	if (read(fd, (char*)&sockfd, (sizeof(evutil_socket_t))) == sizeof(evutil_socket_t))
	{
		if (sockfd < 0)
		{
			WLogError("CEventBaseMgr::PipeReadCallBack::Pipe Read succuss,but sockfd = %d!\n", sockfd);
		}
		else
		{
			CEventBaseMgr* pThisObj = (CEventBaseMgr*)pLibEvThread->pParam;
			pThisObj->OnPipeReadCallback(sockfd, pBase);
		}
	}
	else
	{
		WLogError("CEventBaseMgr::PipeReadCallBack::Pipe Read Error!\n");
		exit(-1);
	}
}

void CEventBaseMgr::OnPipeReadCallback(evutil_socket_t& sockfd, void* arg)
{
	event_base* pBase = (event_base*)arg;
	bufferevent* bev = NULL;
#ifndef WIN32
	if (m_enablessl)
	{

		SSL* ssl=  SSL_new(SSL_CTX_new(SSLv23_method()));
		if(!ssl)
		{
			WLogError("CEventBaseMgr::PipeReadCallBack::SSL_new error!\n");
			exit(-1);
		}
		bufferevent_ssl_state state = BUFFEREVENT_SSL_CONNECTING;
		bev = bufferevent_openssl_socket_new(pBase, sockfd, ssl, state, BEV_OPT_CLOSE_ON_FREE);

	}
#endif
	if (!m_enablessl)
		bev = bufferevent_socket_new(pBase, sockfd, BEV_OPT_CLOSE_ON_FREE);
	if(!bev)
	{
		WLogError("CEventBaseMgr::PipeReadCallBack::bufferevent_socket_new error!\n");
		evutil_closesocket(sockfd);
		exit(-1);
		return ;
	}

	bufferevent_setcb(bev, 
		SocketReadCallBack, 
		/*SocketWirteCallBack*/NULL, 
		SocketEventCallBack, 
		this);

	int iret = bufferevent_enable(bev, EV_READ | EV_PERSIST);
	if (iret != 0)
	{
		WLogError("CEventBaseMgr::PipeReadCallBack::bufferevent_enable error!\n");
		evutil_closesocket(sockfd);
		exit(-1);
	}
}

void CEventBaseMgr::SocketReadCallBack(bufferevent* bev, void* arg)
{
	CEventBaseMgr* pThisObj = (CEventBaseMgr*)arg;
	if(pThisObj)
		pThisObj->OnReadCallback(bev);
}

//读数据处理，此函数被调用，说明数据已经从内核区拷贝到bufferevent的缓冲区
void CEventBaseMgr::OnReadCallback(bufferevent*& bev)
{
	//数据接收,必须确保当前已经把bufferevent中的数据全部读取出来
	char msg[4096];
	size_t len = bufferevent_read(bev, msg, sizeof(msg));
	msg[len] = '\0';

	//请求处理	
	int iRet = -1;
	std::string reply = "Error";
	if(m_pRequestMgr)
		iRet = m_pRequestMgr->HandleRequest(bev, msg, reply);

	//若返回数据不完整，表示需要继续接收,先不回复，等待下次回调
	if(iRet != DATA_DEF)
		bufferevent_write(bev, reply.c_str(), reply.size());
}

void CEventBaseMgr::SocketWirteCallBack(bufferevent* bev, void* arg)
{

}

void CEventBaseMgr::OnWirteCallback(bufferevent*& bev)
{

}

//异常回调处理
void CEventBaseMgr::SocketEventCallBack(struct bufferevent *bev, short event, void *arg)
{
	CEventBaseMgr* pThisObj = (CEventBaseMgr*)arg;
	if(pThisObj)
		pThisObj->OnEventCallback(bev, event);
}

void CEventBaseMgr::OnEventCallback(bufferevent*& bev, short& event)
{
	if (event & BEV_EVENT_ERROR)
	{
		WLogError("some other error\n");
	}
	else if(!(event & BEV_EVENT_EOF))
	{
		WLogError("unknow error!\n");
	}

	//这将自动close套接字和free读写缓冲区	
#ifndef WIN32
	SSL* ssl = bufferevent_openssl_get_ssl(bev);
	if(ssl)
	{
		if(ssl->ctx)
			SSL_CTX_free(ssl->ctx);
		SSL_free(ssl);
	}
#endif
	bufferevent_free(bev);
}

void CEventBaseMgr::TimeoutCallBack(evutil_socket_t fd, short event, void* arg)
{
	CEventBaseMgr* pThisObj = (CEventBaseMgr*)arg;
	if (pThisObj)
	{
		pThisObj->OnTimeoutCallback();
	}
}

void CEventBaseMgr::OnTimeoutCallback()
{
	if (m_pRequestMgr)
	{
		m_pRequestMgr->OnCheckActiveTime();
	}
}