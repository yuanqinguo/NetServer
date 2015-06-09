#include "CReqHandlerObj.h"
#include "CRedisServiceMgr.h"
#include "CSqlServiceMgr.h"
#include "MemoryPoolObj.h"
#include "CHttpParse.h"

#include "wLog.h"
#include "Common.h"


CReqHandlerObj::CReqHandlerObj(void* arg)
{
	m_pArg = arg;
	m_requestStr.clear();
	m_pRedisMgr = CRedisServiceMgr::GetInstance();
	m_pSqlMgr = CSqlServiceMgr::GetInstance();
	m_pHttpParse = NULL;
}

CReqHandlerObj::~CReqHandlerObj()
{
	m_pArg = NULL;
	if(m_pHttpParse)
		delete m_pHttpParse;

	m_pHttpParse = NULL;
}

//此函数被执行表示当前为一个有效的请求，并且，数据接收完整无误
int CReqHandlerObj::OnHandle(std::string& reply)
{
	reply.clear();
	std::string& content = m_pHttpParse->get_content();


	reply = "HTTP/1.1 200 OK\r\n"
			"Date: Thu, 04 Jun 2015 12:56:22 GMT\r\n"
			"Server: nginx\r\n"
			"Content-type: text/html;charset=UTF-8\r\n"
			"Connection: Close\r\n"
			"Content-Length: 17\r\n\r\n"
			"test hello world";
	return SUCCESS;
}

int CReqHandlerObj::CheckData(const std::string& request)
{
	//每次数据流通，更新对象活动时间，避免被检测机制剔除
	UpdateActiveTime();
	m_requestStr += request;

	if(m_pHttpParse)
		m_pHttpParse->parse_request(m_requestStr);
	else
		m_pHttpParse = new CHttpParse(m_requestStr);

	if (m_pHttpParse->get_content().length() == m_pHttpParse->get_content_lenght())
	{
		return DATA_OK;
	}
	else if (m_pHttpParse->get_content().length() < m_pHttpParse->get_content_lenght())
	{
		return DATA_DEF;
	}

	return DATA_ERROR;
}
