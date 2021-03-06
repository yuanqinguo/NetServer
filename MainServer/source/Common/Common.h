#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include<iostream>
#include<string.h>
#ifdef WIN32
#include <objbase.h>
#else
#include <uuid/uuid.h>
#endif
#include <time.h>
#include <string>

/*
SRC000	印章状态正常
SRC001	信息对比校验失败
SRC002	状态校验，已注销
SRC003	状态校验，已锁定
SRC004	过期校验
SRC005	验证平台内部处理异常
*/

#define SRC_ESTATUS_OK				"SRC000"
#define SRC_ESTATUS_FAIL			"SRC001"
#define SRC_ESTATUS_CANCELED		"SRC002"
#define SRC_ESTATUS_LOCKED			"SRC003"
#define SRC_ESTATUS_EXPIRE			"SRC004"
#define SRC_ESTATUS_SERVER_EX		"SRC005"



//log_esign insert 头数据格式
static const char* INSERT_LOG_ESIGN = 
	"INSERT INTO log_esign \
	(lges_uid,LGES_SIGNSN,LGES_DOCHASH,LGES_DOCID,LGES_docname,LGES_sealcode,LGES_sealname,LGES_sealhash,LGES_compname,LGES_compcode,\
	LGES_signdate,LGES_dochashsign,LGES_keysn,LGES_keytype,LGES_certsn,LGES_certhash,LGES_doctype,LGES_docversion,LGES_clientversion,LGES_ip,\
	LGES_mac,LGES_info,LGES_clientos,LGES_DOCPAGES,LGES_signpage,LGES_signx,LGES_signy,LGES_RETURNCODE,LGES_AREACODE,\
	LGES_AREANAME,LGES_LOGDATE) \
	VALUES";

static const char* LOG_ESIGN_FORMAT = 
	"('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',\
	'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',\
	'%s','%s','%s','%s','%s','%s','%s','SRC000','%s',\
	NULL,NOW())";


//log_signrevoke insert 头数据格式
static const char* INSERT_LOG_SIGNREVOKE = 
	"INSERT INTO log_signrevoke \
	(lgsr_uid,lgsr_SIGNSN,lgsr_DOCHASH,lgsr_DOCID,lgsr_docname,lgsr_sealcode,lgsr_sealname,LGSR_SEALHASH,lgsr_compname,lgsr_compcode,\
	lgsr_areaname,lgsr_areacode,lgsr_keysn,lgsr_keytype,LGSR_SIGNHASH,lgsr_signdate,lgsr_clientversion,\
	lgsr_ip,lgsr_mac,lgsr_info,lgsr_clientos,LGSR_RETURNCODE,LGSR_REVOKEDATE) \
	VALUES";

static const char* LOG_SIGNREVOKE_FORMAT = 
	"('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',\
	NULL,\
	'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','SRC000',NOW())";


//log_signverify insert 头数据格式
static const char* INSERT_LOG_SIGNVERIFY = 
	"INSERT INTO log_signverify \
	(lgsv_uid,lgsv_SIGNSN,lgsv_DOCHASH,LGSV_SIGNHASH,lgsv_DOCID,lgsv_docname,lgsv_sealcode,lgsv_sealname,lgsv_sealhash,lgsv_compname,\
	lgsv_compcode,lgsv_signdate,lgsv_clientversion,lgsv_ip,lgsv_mac,lgsv_info,lgsv_clientos,LGSV_SVDATE,LGSV_LEVEL,\
	LGSV_AREACODE,LGSV_AREANAME,LGSV_RETURNCODE) \
	VALUES";
static const char* LOG_SIGNVERIFY_FORMAT = 
	"('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',\
	'%s','%s','%s','%s','%s','%s','%s',NOW(),0,'%s',\
	NULL,'SRC000')";



//eseal insert 头数据格式
static const char* INSERT_ESEAL = 
	"INSERT INTO eseal \
	(ESEL_sealcode,ESEL_sealname,ESEL_HASH,ESEL_compname,ESEL_compcode,ESEL_TYPE,ESEL_AREACODE,\
	ESEL_AREANAME,ESEL_APPLYDATE,ESEL_EXPIREDATE,ESEL_description,ESEL_height,ESEL_width, \
	ESEL_size,ESEL_ext,ESEL_keysn,ESEL_keytype,ESEL_OPERATOR,ESEL_OPERATORUID,ESEL_CERTUID,\
	ESEL_CERTSN,ESEL_CERTHASH,ESEL_STATUS,ESEL_CREATEDATE,ESEL_UPDATEDATE) 	\
	VALUES";

static const char* INSERT_ESEAL_FORMAT = 
	"('%s','%s','%s','%s','%s','%s','%s',\
	NULL,NOW(),\
	(SELECT  (DATE_ADD(NOW(),INTERVAL 3 MONTH))),\
	'%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s',4,NOW(),NOW())";


//log_produceeseal insert 头数据格式
static const char* INSERT_LOG_PRODUCEESEAL =
	"INSERT INTO log_produceeseal \
	(LGPE_UID,LGPE_sealcode,LGPE_sealname,LGPE_SEALHASH,LGPE_compname,LGPE_compcode,LGPE_SEALTYPE,\
	LGPE_keysn,LGPE_keytype,LGPE_puunit,LGPE_puunitcode,LGPE_puname,LGPE_puaccount,\
	LGPE_CERTSN,LGPE_CERTHASH,LGPE_ip,LGPE_mac,LGPE_info,LGPE_clientos,LGPE_clientversion,\
	LGPE_AREACODE,LGPE_AREANAME,LGPE_LEVEL,LGPE_LOGDATE,LGPE_RETURNCODE)\
	VALUES";

static const char* LOG_PRODUCEESEAL_ESEAL_FORMAT = 
	"('%s','%s','%s','%s','%s','%s','%s','%s','%s',\
	'%s','%s','%s','%s','%s','%s','%s','%s',\
	'%s','%s','%s','%s',NULL,0,NOW(),'%s')";


/*

*/
static const char* UPDATE_ESEAL_FORMAT = 
	"UPDATE eseal SET ESEL_STATUS = 4 ,ESEL_HASH = '%s',ESEL_keysn = '%s',ESEL_keytype= '%s',\
	esel_updatedate = NOW(),ESEL_OPERATOR = '%s',ESEL_OPERATORUID='%s' \
	WHERE ESEL_sealcode = '%s';";


static const char* SELECT_ESEAL_FORMAT =
"SELECT CASE WHEN NOW()>esel_expiredate THEN 9  ELSE esel_status END,\
IFNULL(esel_expiredate,''),IFNULL(esel_revokedate,'') FROM ESEAL \
WHERE esel_sealcode = '%s'";




class Common
{
public:
	Common(){};
	~Common(){};
	static void initBase64();

	static void EncodeInput1(/* in */const unsigned char pData[1], /* out */char EncodeBuf[4]);

	static void EncodeInput2(/* in */const unsigned char pData[2], /* out */char EncodeBuf[4]);

	static void EncodeInput3(/* in */const unsigned char pData[3], /* out */char EncodeBuf[4]);

	static std::string Encode(unsigned char* pData, int DataLen);

	static std::string Decode(unsigned char* pData, int DataLen, int* nByte);

	static int GetCRC(const char* pszValue,int nLength);

	static char* GetUid(char *puuid);

	static char* GetDateTime(char *pdate);
	static std::string GetDateTime();
protected:
private:
};

#endif
