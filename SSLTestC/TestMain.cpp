#include "openssl/bio.h"  
#include "openssl/ssl.h"  
#include "openssl/err.h" 

#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#else
#define close(x) closesocket(x)
#endif
#include <cstdio>
#include <string>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"

#define CA_CERT_FILE "client/ca.crt"
#define CLIENT_CERT_FILE "client/client.crt"
#define CLIENT_KEY_FILE "client/client.key"

int main(int argc, char **argv)  
{  
	std::string address = SERVER_IP;
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

	if(argc > 1)
	{
		address = argv[1];
	}

	SSL_METHOD  *meth;  
	SSL_CTX     *ctx;  
	SSL         *ssl;  

	int nFd;  
	int nLen;  
	char szBuffer[1024];  
  
	SSLeay_add_ssl_algorithms();  
	OpenSSL_add_all_algorithms();  
	SSL_load_error_strings();  
	ERR_load_BIO_strings();  

	// 我们使用SSL V3,V2  
	ctx = SSL_CTX_new (SSLv23_method());
	if( ctx == NULL)
	{
		printf("SSL_CTX_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 要求校验对方证书  
	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);  

	// 加载CA的证书
	printf("SSL_CTX_load_verify_locations start!\n");
	if(!SSL_CTX_load_verify_locations(ctx, CA_CERT_FILE, NULL))
	{
		printf("SSL_CTX_load_verify_locations error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 加载自己的证书  
	if(SSL_CTX_use_certificate_file(ctx, CLIENT_CERT_FILE, SSL_FILETYPE_PEM) <= 0)
	{
		printf("SSL_CTX_use_certificate_file error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	//// 加载自己的私钥 
	//printf("SSL_CTX_use_PrivateKey_file start!\n");
	//if(SSL_CTX_use_PrivateKey_file(ctx, CLIENT_KEY_FILE, SSL_FILETYPE_PEM) <= 0)
	//{
	//	printf("SSL_CTX_use_PrivateKey_file error!\n");
	//	ERR_print_errors_fp(stderr);
	//	return -1;
	//}

	//// 判定私钥是否正确  
	//if(!SSL_CTX_check_private_key(ctx))
	//{
	//	printf("SSL_CTX_check_private_key error!\n");
	//	ERR_print_errors_fp(stderr);
	//	return -1;
	//}

	// 创建连接  
	nFd = ::socket(AF_INET, SOCK_STREAM, 0); 

	struct sockaddr_in addr; 
	addr.sin_addr.s_addr = inet_addr(address.c_str());
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);

	if(connect(nFd, (sockaddr *)&addr, sizeof(addr)) < 0)  
	{  
		printf("connect\n"); 
		ERR_print_errors_fp(stderr);
		return -1;  
	}  

	// 将连接付给SSL  
	ssl = SSL_new (ctx);
	if( ssl == NULL)
	{
		printf("SSL_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}
	SSL_set_fd (ssl, nFd);  
	if( SSL_connect (ssl) != 1)
	{
		printf("SSL_new error!\n");
		ERR_print_errors_fp(stderr);
		return -1;
	}

	// 进行操作  
	sprintf(szBuffer, "\nthis is from client+++++++++++++++client send to server");  
	SSL_write(ssl, szBuffer, strlen(szBuffer));  

	// 释放资源  
	memset(szBuffer, 0, sizeof(szBuffer));  
	nLen = SSL_read(ssl,szBuffer, sizeof(szBuffer));  
	fprintf(stderr, "Get Len %d %s ok\n", nLen, szBuffer);  

	SSL_free (ssl);  
	SSL_CTX_free (ctx);  
	close(nFd);  
}  