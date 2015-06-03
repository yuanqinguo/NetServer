#ifndef COMM_STRUCT_H
#define COMM_STRUCT_H

#include <string>


#define REPLY_UNKNOW "unknown error!"
#define REPLY_FAIL "command fail!"
#define REPLY_DATA_DEF "data deficiencies!"
#define REPLY_ERROR "data error!"
#define REPLY_ZONECODE_ERR "areacode invalid!"
#define REPLY_CMD_INVALID "invalid command!"

#define _INSERT_LOG_ESIGN "_INSERT_LOG_ESIGN"
#define _INSERT_LOG_SIGNVERIFY "_INSERT_LOG_SIGNVERIFY"
#define _INSERT_LOG_SIGNREVOKE "_INSERT_LOG_SIGNREVOKE"
#define _INSERT_LOG_PRODUCEESEAL	"_INSERT_LOG_PRODUCEESEAL"
#define _INSERT_ESEAL "_INSERT_ESEAL"
#define _UPDATE_ESEAL "_UPDATE_ESEAL"

//数据包检测检测结果类型
typedef enum 
{
	UNKNOW = -1,	//未知错误
	SUCCESS = 0,	//成功
	FAIL = 1,		//失败
	DATA_OK = 2,	//数据正常
	DATA_DEF = 3,	//数据不足（被分包），表示需要继续接收
	DATA_ERROR = 4,	//数据错误
	ZONECODE_ERR = 5,	//区域编码无效
	CMD_INVALID = 6,	//请求无效	

}HAND_REPLY;

//数据包检测检测结果类型
typedef enum 
{
	E_UNKNOW = -1,		//未知
	E_APPROVED = 1,		//已审批
	E_UNDERTAKED = 2,	//已承接
	E_PRODUCED = 3,		//已制作
	E_PAID = 4,			//已支付
	E_SCRAPPED = 5,		//已报废
	E_CANCELED = 6,		//已注销
	E_LOSS = 7,			//已挂失	
	E_LOCKED = 8,		//已锁定
	E_EXPIRED = 9		//已过期

}EN_ESTATUS;


//请求对象类型
typedef enum
{
	REQ_UNKNOW = -1,
	REQ_SEAL_REQ = 0x05,	//签章请求
	REQ_SEAL_LOG = 0x06,	//签章日志
	REQ_CANCEL_REQ = 0x07,	//撤销请求
	REQ_CANCEL_LOG = 0x08,	//撤销日志
	REQ_VERIFY_REQ = 0x09,	//验证请求
	REQ_CREATE_REQ = 0x10	//生产请求
}REQUEST_TYPE;

//配置信息
typedef struct
{
	std::string st_address;
	std::string st_port;
	std::string st_username;
	std::string st_password;
	std::string st_dbname;
}ConfigInfo;

//通信协议
typedef struct  
{
	int ilen;
	std::string strAreaCode;
	std::string strVersion;
	REQUEST_TYPE eType; //请求类型
	std::string strTime;//时间
	std::string strLicence; //软件许可 
	std::string strData; //请求数据信息
	std::string szDataParam[32];
	std::string strSealCode;
	int iCrc;	//校验
}ComProtocol;

#endif