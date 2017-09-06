#include <iostream>
#include <curl/curl.h>
#include <cstdlib>
#include <list>


#define  HTTP_BODY_MAXLEN 1024*2
#define RECVBUFF_LEN 8*1024
#define URL_LEN 1024
#define CONTENT_LEN 1024

using namespace std;
typedef struct{
	bool isAble;
	int type;
	string url;
	string onlineNum;
}RetValue;


typedef struct{
	int id;
	char url[URL_LEN + 1];
	char content[CONTENT_LEN + 1];
	int type;
}request_t;

typedef struct MemoryStruct{
	char* memory;
	size_t size;
}MemoryStruct;
// typedef struct{
// 	char buffer[RECVBUFF_LEN];
// 	size_t size;
// }recv_buffer_t;

class RequestInfo {
public:
	RequestInfo();
	~RequestInfo();

	void setid(int _id){
		id=_id;
	}
	int getid(){
		return id;
	}
	CURL* get_handle(){
		return curl_handle;
	}
	void set_handle(CURL* handle) {
		curl_handle=handle;
	}
	MemoryStruct* buff(){
		return &chunk;
	}
private:
	int id;
	CURL* curl_handle;
	MemoryStruct chunk;
};

int curl_multi_handles(RetValue* req ,int len,unsigned int TIMEOUT);

class multiCurl
{
public:
	int getResponse(int len, RequestInfo* reqArray);
	int add_request_list( int m, int n, const char* url, const char* content );
	void setTimeOut(unsigned int time);
	int add_curl_handle();
private:
	int request_select();
	void add_request(int id,const char* url, int type,const char* content);
	int add_request_handle(RequestInfo* info);
public:
	multiCurl();
	~multiCurl();
private:
	CURLM* m_curl;
	std::list<request_t> request_list;
	int request_num;
	unsigned int m_TimeOut;
};