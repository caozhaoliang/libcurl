#include <iostream>
#include <curl/curl.h>

using namespace std;

class easyCurl
{
public:
	void setDebug(bool isDebug);
	int http_get(const string& strUrl,const string& header,int TIMEOUT, string& szResponse,const char* pCaPath);
	int http_post(const string& strUrl,const string& header,const string& sBody,int TIMEOUT, string& szResponse,const char* pCaPath);
public:
	easyCurl();
	~easyCurl();

private:
	bool m_bDebug;	
};