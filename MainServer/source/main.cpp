#include "wLog.h"
#include "CMainEventBase.h"
#include "CMgrRequest.h"
#include "CRedisServiceMgr.h"
#include "CSqlServiceMgr.h"
#include "CConfiger.h"
#include "Common.h"
#include "event2/thread.h"

#include <stdio.h>
#include <cstring> 

#define LISTEN_PORT 8080
#define THREAD_NUM 5
#define CONFIG_FILE "server.conf"


//windows 下是单线程的服务
//linux下是多个event_base，多线程，使用管道方式去除了加锁和解锁

/*
* 单例生成
* 单例生成实现已保证线程安全，但建议在主函数中进行生成
*/
void SingletonInit()
{
	CConfiger::GetInstance();
	CMgrRequest::GetInstance();
	CRedisServiceMgr::GetInstance();
	CSqlServiceMgr::GetInstance();
}

/*
* 单例销毁
*/
void SingletonUnInit()
{
	CMgrRequest::DelInstance();
	CRedisServiceMgr::DelInstance();
	CSqlServiceMgr::DelInstance();
	CConfiger::DelInstance();
}

//初始化系统各模块组件
void SystemInit()
{
	SingletonInit();

	if(CConfiger::GetInstance()->OnInit(CONFIG_FILE) != true)
	{
		SingletonUnInit();
		exit(-1);
	}

	if (CConfiger::GetInstance()->GetEnableSSL())
	{
		SSLeay_add_ssl_algorithms();  
		OpenSSL_add_all_algorithms();  
		SSL_load_error_strings();  
		ERR_load_BIO_strings();  
	}

	Common::initBase64();
	CSqlServiceMgr::GetInstance()->OnInit();
	CRedisServiceMgr::GetInstance()->OnInit();

#ifdef WIN32
	//windows初始化网络环境
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR)
	{
		printf("Error at WSAStartup()\n");
		exit(-1);
	}
	printf("Server Running in WONDOWS\n");
#else
	printf("Server Running in LINUX\n");
#endif
}

//反初始化系统各模块组件
void SystemQuit()
{
	SingletonUnInit();
}


int main(int argc, char** argv)  
{ 
	int iListenerPort = LISTEN_PORT;
	int iThreadNum = THREAD_NUM;

	if (argc >= 2 && argv[1])
	{
		iListenerPort = atoi(argv[1]);
	}

	//日志记录初始化
	int port = iListenerPort;
	char buffer[32] = {0};
	sprintf(buffer, "%s_%d", argv[0], iListenerPort);
	WLogInit("./SealServerLog",buffer, 10, 1000, 1024);

	bool IsStart = false;
	CMainEventBase* pMainServer = new CMainEventBase();
	do 
	{
		SystemInit();
		if(pMainServer)
		{
			if (CConfiger::GetInstance()->GetEnableSSL())
			{
				WLogInfo("*******************Server Start enable_ssl!********************\n");
			}
			else
			{
				WLogInfo("*******************Server Start disenable_ssl!********************\n");
			}

			iThreadNum = CConfiger::GetInstance()->GetCfgThreads();
#ifndef WIN32
			if(pMainServer->OnInit(iListenerPort, iThreadNum) != true)
				break;
#else
			if(pMainServer->OnInit(LISTEN_PORT, 1) != true)
				break;
#endif
			pMainServer->OnStart();		//event_base_dispatch 阻塞
			pMainServer->OnStop();
			IsStart = true;
		}
		else
		{
			printf("main::new CMainEventBase error!\n");
		}
	} while (0);

	if(IsStart)
	{
		SystemQuit();
	}

	delete pMainServer;
	pMainServer = NULL;

	return 0;
} 