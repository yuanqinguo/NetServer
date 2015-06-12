#include "CEventBaseMgr.h"
#include "CMgrRequest.h"
#include "wLog.h"
#include "CConfiger.h"

#include <cstdlib>
#include <errno.h>

#define CA_CERT_FILE "server/ca.crt"
#define SERVER_CERT_FILE "server/server.crt"
#define SERVER_KEY_FILE "server/server.key"

CEventBaseMgr::CEventBaseMgr(const int iNum)
{
	m_iThreadnum = iNum;
	m_isStop = false;
	m_pLibEvThreads = NULL;
	m_pRequestMgr = CMgrRequest::GetInstance();

	LibEvInit();
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

SSL* CEventBaseMgr::CreateSSL(evutil_socket_t& fd)
{
	if (!CConfiger::GetInstance()->GetEnableSSL())
	{
		return NULL;
	}

	SSL_CTX* ctx = NULL;  
	SSL* ssl = NULL; 
	ctx = SSL_CTX_new (SSLv23_method());
	if( ctx == NULL)
	{
		printf("SSL_CTX_new error!\n");
		return NULL;
	}

	// 要求校验对方证书  
	SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);  

	// 加载CA的证书  
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		SSL_CTX_free(ctx);
		printf("SSL_CTX_load_verify_locations error!\n");
		return NULL;
	}

	// 加载自己的证书  
	if(SSL_CTX_use_certificate_file(ctx, SERVER_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		SSL_CTX_free(ctx);
		printf("SSL_CTX_use_certificate_file error!\n");
		return NULL;
	}

	// 加载自己的私钥  
	if(SSL_CTX_use_PrivateKey_file(ctx, SERVER_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		SSL_CTX_free(ctx);
		printf("SSL_CTX_use_PrivateKey_file error!\n");
		return NULL;
	}

	// 判定私钥是否正确  
	if(!SSL_CTX_check_private_key(ctx))
	{
		SSL_CTX_free(ctx);
		printf("SSL_CTX_check_private_key error!\n");
		return NULL;
	}

	// 将连接付给SSL  
	ssl = SSL_new (ctx);
	if(!ssl)
	{
		SSL_CTX_free(ctx);
		printf("SSL_new error!\n");
		return NULL;
	}

	SSL_set_fd(ssl, fd);  
	bool isContinue = true;
	while(isContinue)
	{
		isContinue = false;
		if(SSL_accept(ssl) != 1)
		{
			int icode = -1;
			int iret = SSL_get_error(ssl, icode);
			if (iret == SSL_ERROR_WANT_READ ||
				iret == SSL_ERROR_WANT_WRITE)
			{
				isContinue = true;
			}
			else
			{
				SSL_CTX_free(ctx);
				SSL_free(ssl);
				ctx = NULL;
				ssl = NULL;

				break;
			}
		}
		else
			break;
	}

	return ssl;
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
	if(sockfd < 0)
	{
		static int iAcptCount;
		WLogError("CEventBaseMgr::OnPipeReadCallback::sockfd < 0\n");
		return;
	}

	evutil_make_socket_nonblocking(sockfd);

	SSL* ssl = CreateSSL(sockfd);
	if(CConfiger::GetInstance()->GetEnableSSL() && !ssl)
	{
		WLogError("CMainEventBase::OnAcceptCallback::CreateSSL error! ssl NULL ");
		return ;
	}
	struct event *ev = event_new(NULL, -1, 0, NULL, NULL);

	//将动态创建的结构体作为event的回调参数
	EventCtx* pCtx = new EventCtx();
	pCtx->pEvent = ev;
	pCtx->pSsl = ssl;
	pCtx->sock = sockfd;

	event_assign(ev, pBase, sockfd, EV_READ | EV_PERSIST,
		EventCallBack, (void*)pCtx);

	event_add(ev, NULL);
}

void CEventBaseMgr::EventCallBack(evutil_socket_t fd, short event, void* arg)
{
	int iret = -1;
	EventCtx* pCtx= (EventCtx*)arg;
	if(pCtx)
	{
		bool enablessl = CConfiger::GetInstance()->GetEnableSSL();
		CMgrRequest* pMgr = CMgrRequest::GetInstance();
		struct event* pEvent = pCtx->pEvent;
		SSL* pssl = pCtx->pSsl;

		char buffer[4096] = {0};
		int len = 0;
		if (enablessl)
		{
			len = SslRecv(pssl, buffer, sizeof(buffer));
		}
		else
		{
			len =	NormalRecv(fd, buffer, sizeof(buffer));
		}

		if (len <= 0)//客户端关闭连接，或者socket出现错误
		{
			EventCtx_free(pCtx);
			return;
		}

		std::string reply = "ERROR";
		iret = pMgr->HandleRequest(pCtx, buffer, reply);
		if(iret != DATA_DEF)//若为数据不足，则暂时不回复，等待下次回调
		{
			if (enablessl)
			{
				len = SslSend(pssl, reply.c_str(), reply.length());
			}
			else
			{
				len = NormalSend(fd, reply.c_str(), reply.length());
			}

			if (len < 0)
			{
				WLogError("CEventBaseMgr::EventCallBack::send error!\n");
			}
		}
	}
	else
	{
		WLogError("CEventBaseMgr::EventCallBack::EventCtx is NULL error!\n");
	}
}


int CEventBaseMgr::SslRecv(SSL* ssl, char* buffer, int ilen)
{
	int ires = 0, count = 0;;
	bool isCoutinue = true;
	while (isCoutinue)
	{
		ires = SSL_read(ssl, buffer + count, ilen - count);
		int nRes = SSL_get_error(ssl, ires);
		if(nRes == SSL_ERROR_NONE)
		{
			if(ires > 0)
			{
				count += ires;
				if (count >= ilen)
				{
					break;
				}
				continue;
			}
		}
		else
		{
			break;
		}
	}

	return count;
}

int CEventBaseMgr::NormalRecv(evutil_socket_t& fd, char* buffer, int ilen)
{
	int ires = 0, count = 0;;
	bool isCoutinue = true;
	while (isCoutinue)
	{
		ires = recv(fd, buffer+count, ilen-count, 0);
		if (ires < 0)
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;	//没有读到数据
			}
			else
			{
				break;	//读取失败
			}
		}
		else if (ires == 0)
		{
			break;	//读取完成，或者客户端关闭连接
		}
		else
		{
			count += ires;	//读到数据
			if (count >= ilen)	//缓冲区不够大，暂时不处理
			{
				break;
			}
		}
	}

	return count;
}

int CEventBaseMgr::SslSend(SSL* ssl, const char* buffer, int ilen)
{
	int ires = 0, count = 0;;
	bool isCoutinue = true;
	while (isCoutinue)
	{
		ires = SSL_write(ssl, buffer + count, ilen - count);
		int nRes = SSL_get_error(ssl, ires);
		if(nRes == SSL_ERROR_NONE)
		{
			if(ires > 0)
			{
				if (count >= ilen)
				{
					break;
				}
				count += ires;
				continue;
			}
		}
		else if (nRes == SSL_ERROR_WANT_READ)
		{
			continue;
		}
		else
		{
			break;
		}
	}

	return count;
}

int CEventBaseMgr::NormalSend(evutil_socket_t& fd, const char* buffer, int ilen)
{
	int ires = 0, count = 0;;
	bool isCoutinue = true;
	while (isCoutinue)
	{
		ires = send(fd, buffer+count, ilen-count, 0);
		if (ires < 0)
		{
			if(errno == EWOULDBLOCK || errno == EAGAIN)
			{
				continue;	//内核缓冲区空间不足
			}
			else
			{
				break;	//写入失败
			}
		}
		else
		{
			count += ires;	//写入ires数据
			if (count >= ilen)	//数据写入完成
			{
				break;
			}
		}
	}

	return count;
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