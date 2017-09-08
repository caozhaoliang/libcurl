#include "easyCurl.hpp"
#include <cstring>
#include <cstdlib>
#include <vector>

#define HTTP_URL_MAXLEN 1024

easyCurl::easyCurl(){
	// m_curl = getInstance();
	m_bDebug = false;
}
easyCurl::~easyCurl(){
	// curl_easy_cleanup(m_curl);
	// m_curl = NULL;
}

static int OnDebug(CURL* curl ,curl_infotype itype,char* pData,size_t szie,void *p) {
	if( itype == CURLINFO_TEXT ) {
		printf("[TEXT]%s\n",pData );
	}
	else if(itype == CURLINFO_HEADER_IN)  
    {  
        printf("[HEADER_IN]%s\n", pData);  
    }  
    else if(itype == CURLINFO_HEADER_OUT)  
    {  
        printf("[HEADER_OUT]%s\n", pData);  
    }  
    else if(itype == CURLINFO_DATA_IN)  
    {  
        printf("[DATA_IN]%s\n", pData);  
    }  
    else if(itype == CURLINFO_DATA_OUT)  
    {  
        printf("[DATA_OUT]%s\n", pData);  
    }  
    return 0;
}

static size_t onWriteData(void* buffer, size_t size, size_t nmemb, void* lpVoid)  
{  
    string* str = dynamic_cast<string*>((string *)lpVoid);  
    if( NULL == str || NULL == buffer )  
    {  
        return -1;  
    }  
    char* pData = (char*)buffer;  
    str->append(pData, size * nmemb);  
    return nmemb;  
}
size_t write_data(void *ptr, size_t size, size_t nmemb, void* stream)   
{  
	string data((const char*)ptr,(size_t) size*nmemb );
	*((stringstream*) stream) << data <<endl;
	return size*nmemb;
}

/*  libcurl write callback function */  
size_t write_data1(void *ptr, size_t size, size_t nmemb, FILE* stream)   
{  
    size_t written = fwrite(ptr, size, nmemb, stream);  
    return written;  
  
    //在这里可以把下载到的数据以追加的方式写入文件  
    //FILE* fp = NULL;  
    //fopen_s(&fp, "c:\\test.dat", "ab+");//一定要有a, 否则前面写入的内容就会被覆盖了  
    //size_t nWrite = fwrite(ptr, nSize, nmemb, fp);  
    //fclose(fp);  
    //return nWrite;  
} 

void easyCurl::setDebug(bool isDebug){
	m_bDebug = isDebug;
}
int easyCurl::http_get(const string& strUrl,const string& header,int TIMEOUT, string& szResponse,const char* pCaPath){
	if( strUrl.empty() || strUrl.length() < 5 || strUrl.length() > HTTP_URL_MAXLEN ){
		return -1;
	}
	CURLcode res;
	CURLM* m_curl = curl_easy_init();
	if( m_curl == NULL ){
		return CURLE_FAILED_INIT;
	}
	struct curl_slist *hds = NULL;
	if( !header.empty() && header.length() != 0 ) {
		hds = curl_slist_append(hds, header.c_str()); 
		curl_easy_setopt(m_curl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, hds);  
	}
	if( m_bDebug ) {
		curl_easy_setopt(m_curl,CURLOPT_VERBOSE,1);
		curl_easy_setopt(m_curl,CURLOPT_DEBUGFUNCTION,OnDebug);
	}
	curl_easy_setopt(m_curl,CURLOPT_URL,strUrl.c_str());
	curl_easy_setopt(m_curl,CURLOPT_READFUNCTION,NULL);
	curl_easy_setopt(m_curl,CURLOPT_WRITEFUNCTION,onWriteData);
	curl_easy_setopt(m_curl,CURLOPT_WRITEDATA,(void*)&szResponse);
	curl_easy_setopt(m_curl,CURLOPT_NOSIGNAL,1L);
	if( NULL==pCaPath ) {
		curl_easy_setopt(m_curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST,0L);
	}else{
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);  
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, TIMEOUT);  
    res = curl_easy_perform(m_curl);  
    if( res != CURLE_OK ) {
    	std::cout << "curl_easy_perform err:%s"<<curl_easy_strerror(res);
    }
    curl_easy_cleanup(m_curl);
    curl_slist_free_all(hds);  
    return res;

}

int easyCurl::download(const string url,string& outStream){
	stringstream out;
	CURL* curl = curl_easy_init();
	
	curl_easy_setopt(curl,CURLOPT_URL,url.c_str());
	curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
	curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
	curl_easy_setopt(curl,CURLOPT_ACCEPT_ENCODING,"deflate");
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION, write_data);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,&out);
	CURLcode res = curl_easy_perform(curl);
	if(res!=CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",curl_easy_strerror(res) );
		return -1;
	}
	outStream = out.str();
	return 0;

}

int easyCurl::http_post(const string& strUrl,const string& header,const string& sBody,int TIMEOUT, string& szResponse,const char* pCaPath){
	if( strUrl.empty() || strUrl.length() < 5 || strUrl.length() > HTTP_URL_MAXLEN ){
		return -1;
	}
	CURLcode res;
	CURL* m_curl = curl_easy_init();
	if( m_curl == NULL ){
		return CURLE_FAILED_INIT;
	}
	struct curl_slist *hds = NULL;
	if( !header.empty() && header.length() != 0 ) {
		hds = curl_slist_append(hds, header.c_str()); 
		curl_easy_setopt(m_curl, CURLOPT_HEADER, 0L);
		curl_easy_setopt(m_curl, CURLOPT_HTTPHEADER, hds);  
	}
	if( m_bDebug ) {
		curl_easy_setopt(m_curl,CURLOPT_VERBOSE,1);
		curl_easy_setopt(m_curl,CURLOPT_DEBUGFUNCTION,OnDebug);
	}
	curl_easy_setopt(m_curl,CURLOPT_URL,strUrl.c_str());
	curl_easy_setopt(m_curl, CURLOPT_POST, 1);  
    curl_easy_setopt(m_curl, CURLOPT_POSTFIELDS, sBody.c_str());  
	curl_easy_setopt(m_curl,CURLOPT_READFUNCTION,NULL);
	curl_easy_setopt(m_curl,CURLOPT_WRITEFUNCTION,onWriteData);
	curl_easy_setopt(m_curl,CURLOPT_WRITEDATA,(void*)&szResponse);
	curl_easy_setopt(m_curl,CURLOPT_NOSIGNAL,1L);
	if( NULL==pCaPath ) {
		curl_easy_setopt(m_curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYHOST,0L);
	}else{
		curl_easy_setopt(m_curl, CURLOPT_SSL_VERIFYPEER, 1L);  
        curl_easy_setopt(m_curl, CURLOPT_CAINFO, pCaPath);
	}
	curl_easy_setopt(m_curl, CURLOPT_CONNECTTIMEOUT, TIMEOUT);  
    curl_easy_setopt(m_curl, CURLOPT_TIMEOUT, TIMEOUT);  
    res = curl_easy_perform(m_curl);  
    if( res != CURLE_OK ) {
    	std::cout << "curl_easy_perform err:%s"<<curl_easy_strerror(res);
    }
    curl_easy_cleanup(m_curl);
    curl_slist_free_all(hds); 
    return res;

}
