#ifndef CHTTP_PARSE_H
#define CHTTP_PARSE_H

#include <string>
#include <vector>
#include "HttpCommon.h"

class CHttpParse
{
public:
	CHttpParse(std::string& request);

	~CHttpParse();

	std::string get_version() { return m_version; }
	CON_TYPE get_connection() { return m_connection; }

	int get_content_lenght() { return m_content_lenght; }
	std::string& get_content() { return m_content; }
protected:
	void parse_request();

	int parse_line_data_type();

	int parse_line(std::string& keys, std::string& str);

	void parse_header(std::string& retstr);

	void get_line(std::string& retstr); 

	void ToUpper(std::string& str);

	void SplitString(std::string s, char splitchar, std::vector<std::string>& vec);
private:
	HTTP_METHOD m_httpMethod;
	CON_TYPE m_connection;

	std::string m_host;
	std::string m_version;
	std::string m_user_agent;
	std::string m_referer;

	std::string m_accept_encoding;
	std::string m_accept;
	std::string m_language;

	int m_content_lenght;
	std::string m_content;

	std::string& m_requestData;
	bool m_bHeaderDown;
};


#endif //CHTTP_PARSE_H