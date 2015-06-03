#include"Common.h"

char* m_EncodeTable = ((char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
char m_DecodeTable[256];

void Common::initBase64()
{
	// 建立解码表.
	for( int i = 0; i < 256; i++ )
		m_DecodeTable[i] = -1;								// 非法数字.
	for( int i = 0; i < 64; i++ )
	{
		m_DecodeTable[ m_EncodeTable[ i ] ]			= i;
		m_DecodeTable[ m_EncodeTable[ i ] | 0x80 ]	= i;	// 忽略8th bit.
		m_DecodeTable[ '=' ]						= -1;
		m_DecodeTable[ '=' | 0x80 ]					= -1;
	}
}

void Common::EncodeInput1(/* in */const unsigned char pData[1], /* out */char EncodeBuf[4])
{
    EncodeBuf[0] = m_EncodeTable[(pData[0] & 0xFC) >> 2];
    EncodeBuf[1] = m_EncodeTable[((pData[0] & 0x03) << 4)];
    EncodeBuf[2] = '=';
	EncodeBuf[3] = '=';
}

void Common::EncodeInput2(/* in */const unsigned char pData[2], /* out */char EncodeBuf[4])
{
	EncodeBuf[0] = m_EncodeTable[(pData[0] & 0xFC) >> 2];
	EncodeBuf[1] = m_EncodeTable[((pData[0] & 0x03) << 4) | ((pData[1] & 0xF0) >> 4)];
	EncodeBuf[2] = m_EncodeTable[((pData[1] & 0x0F) << 2)];
	EncodeBuf[3] = '=';
}

void Common::EncodeInput3(/* in */const unsigned char pData[3], /* out */char EncodeBuf[4])
{
	EncodeBuf[0] = m_EncodeTable[pData[0] >> 2];
	EncodeBuf[1] = m_EncodeTable[((pData[0] << 4) | (pData[1] >> 4)) & 0x3F];
	EncodeBuf[2] = m_EncodeTable[((pData[1] << 2) | (pData[2] >> 6)) & 0x3F];
	EncodeBuf[3] = m_EncodeTable[pData[2] & 0x3F];
}

std::string Common::Encode(unsigned char* pData, int DataLen)
{
	std::string strEncode		= "";
	char EncodeBuf[5]		= { 0 };
	unsigned char ch[3]		= { 0 };
	int LineLen				= 0;

	for(int i = 0 ; i < (int)(DataLen / 3); i++)
	{
        ch[0] = *pData ++;
        ch[1] = *pData ++;
        ch[2] = *pData ++;
		EncodeInput3(ch, EncodeBuf);
		strEncode += EncodeBuf;

		if(LineLen += 4, LineLen == 76)
		{
			strEncode += "\r\n";
			LineLen = 0;
		}
	}

	//对剩余数据进行编码
	int iMod = DataLen % 3;
	if(iMod == 1)
	{
        memset(EncodeBuf, 0, 5);
        ch[0] = *pData++;
		EncodeInput1(ch, EncodeBuf);
		strEncode += EncodeBuf;
	}
	else if(iMod == 2)
	{
		memset(EncodeBuf, 0, 5);
		ch[0] = *pData++;
		ch[1] = *pData++;
		EncodeInput2(ch, EncodeBuf);
		strEncode += EncodeBuf;
	}
	return strEncode;
}


std::string Common::Decode(unsigned char* pData, int DataLen, int* nByte)
{
	if(nByte != 0)
		*nByte = 0;

	std::string strDecode	= "";
    int nValue			= 0;
    int i				= 0;

	while (i < DataLen)
	{
		if (*pData != '\r' && *pData != '\n')
		{
			nValue		 = m_DecodeTable[*pData ++] << 18;
			nValue		+= m_DecodeTable[*pData ++] << 12;
			// nValue		 = GetDecodeChar(*pData ++) << 18;
			// nValue		+= GetDecodeChar(*pData ++) << 12;
			strDecode	+=(nValue & 0x00FF0000) >> 16;
			if(nByte != 0)
				++ (*nByte);

			if (*pData != '=')
			{
				nValue		+= m_DecodeTable[*pData++] << 6;
				//nValue		+= GetDecodeChar(*pData++) << 6;
				strDecode	+= (nValue & 0x0000FF00) >> 8;
				if(nByte != 0)
					++ (*nByte);

				if (*pData != '=')
				{
					nValue		+= m_DecodeTable[*pData++];
					// nValue		+= GetDecodeChar(*pData++);
					strDecode	+= nValue & 0x000000FF;
					if(nByte != 0)
						++ (*nByte);
				}
			}
			i += 4;
		}
		else // 回车换行,跳过
		{
			pData ++;
			i ++;
		}
	}
	return strDecode;
}

/********************************************************************/
/**	功能:	获得校验码*/
/**	参数:			*/
/**		pszValue--校验的数据				*/
/**		nLength--数据长度 				*/
/********************************************************************/
int Common::GetCRC(const char* pszValue,int nLength)
{
	int i,j;
	unsigned int icrc = 0;
	char bTemp = 0;
	char bHight = 255;
	char bLow = 255;
	int iFlag = 0;
	icrc = bHight*256+bLow;
	i = 0;
	while(i < nLength)
	{
		bTemp = pszValue[i];
		bHight = icrc/256;
		icrc = bTemp^bHight;
		j=0;
		iFlag = 0;
		while(j<8)
		{
			iFlag = icrc % 2;
			icrc = icrc>>1;
			if(iFlag)
			{
				icrc = icrc^40961;
			}
			j++;
		}
		i++;
	}
	return icrc;
}

/********************************************************************/
/**	功能:	获得校验码												*/
/**	参数:															*/
/**		pszValue--校验的数据											*/
/**		nLength--数据长度 											*/
/********************************************************************/
char* Common::GetUid(char *puuid)
{
#ifdef WIN32
	GUID guid;
	CoCreateGuid(&guid);
	char buf[64] = {0};
	_snprintf_s(
		buf,
		sizeof(buf),
		"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data2, guid.Data2, guid.Data3,
		guid.Data2, guid.Data2, guid.Data3,
		guid.Data2, guid.Data2,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);

	memcpy(puuid, buf, strlen(buf));
#else
	uuid_t uu;
	uuid_generate(uu);
	sprintf(puuid,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",
		uu[0],uu[1],uu[2],uu[3],uu[4],
		uu[5],uu[6],uu[7],uu[8],uu[9],
		uu[10],uu[11],uu[12],uu[13],
		uu[14],uu[15]);
#endif
	return puuid;
}

char* Common::GetDateTime(char *pdate)
{
	char buf[32] = {0};
#ifdef WIN32
	time_t t;
	tm *tp;
	t = time(NULL);
	tp = localtime(&t);
	sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d", 
		tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday
		,tp->tm_hour, tp->tm_min, 
		tp->tm_sec);
#else
	time_t now;
	time(&now);
	struct tm timenow;
	localtime_r(&now, &timenow);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timenow);
#endif

	memcpy(pdate, buf, strlen(buf));

	return pdate;
}

std::string Common::GetDateTime()
{
	char buf[32] = {0};
#ifdef WIN32
	time_t t;
	tm *tp;
	t = time(NULL);
	tp = localtime(&t);
	sprintf(buf, "%04d%02d%02d%02d%02d%02d", 
		tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday
		,tp->tm_hour, tp->tm_min, 
		tp->tm_sec);
#else
	time_t now;
	time(&now);
	struct tm timenow;
	localtime_r(&now, &timenow);
	strftime(buf, sizeof(buf), "%Y%m%d%H%M%S", &timenow);
#endif

	return buf;
}
