#ifndef HTTP_COMMON_H
#define HTTP_COMMON_H

//http方法类型
typedef enum
{
	METHOD_UNKNOW = -1,
	METHOD_GET,
	METHOD_POST,
	METHOD_HEADER
}HTTP_METHOD;

//链接动作类型
typedef enum
{
	CON_UNKNOW = -1,
	CON_CLOSE = 0,
	CON_KEEP_LIVE = 1
}CON_TYPE;

#endif //HTTP_COMMON_H