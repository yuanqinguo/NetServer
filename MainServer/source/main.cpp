#include "wLog.h"
#include "CMainEventBase.h"
#include "CMgrRequest.h"
#include "CRedisServiceMgr.h"
#include "CSqlServiceMgr.h"
#include "CRcdServiceMgr.h"
#include "CConfiger.h"
#include "Common.h"
#include "event2/thread.h"

#include <stdio.h>
#include <cstring> 

#define LISTEN_PORT 11181
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
	if (CConfiger::GetInstance()->GetEnableRecord())
		CRcdServiceMgr::DelInstance();
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
	Common::initBase64();
	CSqlServiceMgr::GetInstance()->OnInit();
	CRedisServiceMgr::GetInstance()->OnInit();
	if (CConfiger::GetInstance()->GetEnableRecord())
	{
		CRcdServiceMgr::GetInstance();
	}
}

//反初始化系统各模块组件
void SystemQuit()
{
	SingletonUnInit();
}


int main(int argc, char** argv)  
{ 
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

	int iListenerPort = LISTEN_PORT;
	int iThreadNum = THREAD_NUM;
	bool enablessl = false;
	if (argc >= 2 && argv[1])
	{
		iListenerPort = atoi(argv[1]);
	}
	if (argc >= 3 && argv[2])
	{
		int iEnable = atoi(argv[2]);
		enablessl = (iEnable == 1 ? true : false);
	}

	//日志记录初始化
	int port = iListenerPort;
	char buffer[32] = {0};
	sprintf(buffer, "%s%d", argv[0], iListenerPort);
	WLogInit("./SealServerLog",buffer, 10, 1000, 1024);

	
	bool IsStart = false;
	CMainEventBase* pMainServer = new CMainEventBase();
	do 
	{
		SystemInit();
		if(pMainServer)
		{
			iThreadNum = CConfiger::GetInstance()->GetCfgThreads();
			printf("Main::iThreadNum = %d\n", iThreadNum);
#ifndef WIN32
			if(pMainServer->OnInit(iListenerPort, iThreadNum, enablessl) != true)
				break;
#else
			if(pMainServer->OnInit(LISTEN_PORT, 1) != true)
				break;
#endif
			if (CConfiger::GetInstance()->GetEnableRecord())
			{
				CRcdServiceMgr::GetInstance()->OnStart();
			}
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