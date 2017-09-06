#include "multiCurl.hpp"
#include <cstring>


static size_t onWriteData(void* contents, size_t size, size_t nmemb, void* userp)  
{  
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
	mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
	if (mem->memory == NULL) {
    	/* out of memory! */ 
    	// LOG(ERROR) << "not enough memory (realloc returned NULL)\n";
    	return 0;
	}
	if(realsize == 0 ) {
  		// LOG(ERROR) << "response data's length is 0";
  		return 0;
	}
 
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

multiCurl::multiCurl(){
	m_curl = curl_multi_init();
	request_num = 0;
	request_list.clear();
}
multiCurl::~multiCurl(){
	if(m_curl){
		curl_multi_cleanup(m_curl);
	}
	m_curl = NULL;
	request_list.clear();
}


RequestInfo::RequestInfo()
{
	curl_handle = NULL;
    chunk.memory = (char*)malloc(1);
	if( !chunk.memory) {
		/*game over*/
	}
	chunk.size = 1;
	// recv_buff.size = 1L;
	// recv_buff.buffer[0] = '\0';
}

RequestInfo::~RequestInfo()
{
	if (curl_handle) 
		curl_easy_cleanup(curl_handle);
	curl_handle = NULL;
    free(chunk.memory);
	// recv_buff.size = 0;
}

int multiCurl::add_request_handle( RequestInfo* req) {
	if(!req || !m_curl ) {
		delete req;
		return -1;
	}
	CURLMcode ret ;
	curl_multi_add_handle(m_curl,req->get_handle());
	ret = curl_multi_perform(m_curl, &request_num);
	if( ret != CURLM_OK &&  ret != CURLM_CALL_MULTI_PERFORM){
		std::cout<<"add request failed ret="<<ret<<std::endl;
		return -1;
	}
	return 0; 
}

int multiCurl::add_request_list( int m, int n, const char* url, const char* content ) {
	// request_list.push_back(req);
	if( n==1 && content ==NULL ){
		std::cout<<"content is null"<<std::endl;
		return -1;
	}
	request_t info;
	memset(&info,0,sizeof(info));
	info.type = n;
	info.id = m;
	if( !url || strlen(url) >= URL_LEN ){
		if(url){
			//
			std::cout<<"url length err"<<std::endl;
		}else{
			//
			std::cout<<"url is null"<<std::endl;
		}
		return -1;
	}
	
	strncpy(info.url,url,strlen(url));
	if(content){
		strncpy(info.content,content,strlen(content) );
	}
	request_list.push_back(info);
}

void multiCurl::add_request(int id, const char* url, int type,const char* content ) {
	CURLcode ret;
	RequestInfo* pRequest = new RequestInfo();
	pRequest->setid(id);
	pRequest->set_handle(curl_easy_init());

	curl_easy_setopt(pRequest->get_handle(),CURLOPT_URL,url);

	curl_easy_setopt(pRequest->get_handle(),CURLOPT_HEADER,0L);

	// struct curl_slist * hds=NULL;
	// hds = curl_slist_append(hds,"Content-Type: application/json; charset=utf-8");
	// curl_easy_setopt(pRequest->get_handle(), CURLOPT_HTTPHEADER, hds);
	
	curl_easy_setopt(pRequest->get_handle(), CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(pRequest->get_handle(), CURLOPT_SSL_VERIFYHOST, 0L);

	if( type == 1 ) {
		curl_easy_setopt(pRequest->get_handle(), CURLOPT_CUSTOMREQUEST, "POST");
		curl_easy_setopt(pRequest->get_handle(), CURLOPT_POST, 1L );
		curl_easy_setopt(pRequest->get_handle(), CURLOPT_POSTFIELDS, content);
	}
	curl_easy_setopt(pRequest->get_handle(), CURLOPT_TIMEOUT_MS, m_TimeOut*1000); 
	curl_easy_setopt(pRequest->get_handle(), CURLOPT_PRIVATE, pRequest);

	curl_easy_setopt(pRequest->get_handle(), CURLOPT_WRITEFUNCTION, onWriteData);

	curl_easy_setopt(pRequest->get_handle(), CURLOPT_WRITEDATA, (void*)pRequest->buff());

	add_request_handle(pRequest);
	// curl_slist_free_all(hds);
	return;
}


int multiCurl::add_curl_handle(){
	while(request_list.size() > 0 ) {
		request_t &info = request_list.front();
		add_request(info.id, info.url, info.type, info.content );
		
		request_list.pop_front();
	}
	return 0;
}


int multiCurl::request_select()
{
	CURLMcode ret;
	struct timeval timeout_tv;
	do{
		int max_fd = -1;
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;
		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		timeout_tv.tv_sec = 0;
		timeout_tv.tv_usec = 500000;

	
		ret = curl_multi_fdset(m_curl, &fd_read, &fd_write, &fd_except, &max_fd);
		if (ret != CURLM_OK) {
			return -1;
		}
		if (max_fd >= 0) {
			int rc = select(max_fd + 1, &fd_read, &fd_write, &fd_except, &timeout_tv);
			switch (rc) {
			case -1:
				return -1;
			case 0:
			default:
				curl_multi_perform(m_curl, &request_num);       
				break;
			}
		}
		else {
		    //important request_num
			curl_multi_perform(m_curl, &request_num);       
		}
	}while(request_num);
	return 0;
}

int multiCurl::getResponse(int len, RequestInfo* reqArray){
	
	request_select();
	// 
	CURLMsg* msg;
	int msgs_left;
	// int response_code;
	// reqArray = new RequestInfo[len];

	while( (msg = curl_multi_info_read(m_curl,&msgs_left)) ) {
		// RequestInfo rInfo;
		if(msg->msg == CURLMSG_DONE ) {
			CURL* curl = msg->easy_handle;
			RequestInfo* p_Request = NULL;
			curl_easy_getinfo(curl,CURLINFO_PRIVATE,&p_Request);
			curl_multi_remove_handle(m_curl,curl);
			if( p_Request ) {
				string res;
				res.assign(p_Request->buff()->memory,p_Request->buff()->size);
				std::cout<<"id:"<<p_Request->getid()<<";res:"<<res<<std::endl;
				// reqArray[p_Request->getid()] = *p_Request;
				delete p_Request;
				p_Request=NULL;
			}
		}else{
			std::cout<<"nuknown err"<< std::endl;
		}
		// RequestInfo* req;
		// curl_easy_getinfo( msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code );
		// curl_easy_getinfo( msg->easy_handle, CURLINFO_PRIVATE, &req);
		// reqArray[req.getid()] = req;
		// if (CURLMSG_DONE == msg->msg && response_code == 200 ) {
		// 	/* this is a able value */
		// 	reqArray[req.http_id].isAble = true;
		// }
	}
	return msgs_left;
}
void multiCurl::setTimeOut(unsigned int mtime) {
	m_TimeOut = mtime;
}
int curl_multi_handles(RetValue* req ,int len,unsigned int TIMEOUT) {
	
	multiCurl* mCurl = new multiCurl();
	mCurl->setTimeOut(TIMEOUT);
	string sData = "{\"sessionid\":\"Z\"}";
	for(int i = 0; i < len; i++){
		if( req[i].isAble && req[i].type == 201 ){
			mCurl->add_request_list(i, 1, req[i].url.c_str(), sData.c_str() );
		}else{
			mCurl->add_request_list(i, 0, req[i].url.c_str(), (const char*)NULL );
		}
	}

	mCurl->add_curl_handle();
	RequestInfo* reqArray=NULL;
	mCurl->getResponse(len,reqArray);
	// for(int i = 0; i < len; ++i ) {
	// 	string str(reqArray[i].buff()->buffer);
	// 	std::cout<<"url:"<<req[i].url<<"; res:"<<str<<std::endl;
	// }

	delete[] reqArray;


	delete mCurl;
	mCurl = NULL;
	return 0;
}