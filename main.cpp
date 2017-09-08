#include <iostream>
#include "easyCurl.hpp"
// #include "multiCurl.hpp"


int main(int argc,char* argv[])
{
	/*for easy test*/
	// string url = "https://10.101.222.27:9801/api/trade/ptjy/ptyw/cxxtzt";  // http://<ip>/api/trade/ptjy/ptyw/cxxtzt
	// string szResponse;
	// easyCurl curl;
	// string header = "Content-Type: application/json; charset=utf-8";
	// string sBody = "{\"sessionid\":\"Z\"}";
	// curl.http_post(url,header,"{\"sessionid\":\"Z\"}",30,szResponse,NULL);
	// std::cout<< "szResponse:"<< szResponse<<endl;
	// return 0;
	/*for easy download*/
	easyCurl curl;
	string res="";
	curl.download("https://stackoverflow.com",res);
	cout<<res<<endl;

	/*for multi test*/
	// if(argc < 3){
	// 	return -1;
	// }
	// string url = argv[1]; 
	// multiCurl mCurl;
	// int len = 10;
	// unsigned int TIMEOUT = 30;
	// RetValue* ret = new RetValue[len];
	// for(int i = 0; i < 3; ++i) {
	// 	RetValue mret;
	// 	mret.type = atoi(argv[2]);
	// 	mret.isAble=true;
	// 	mret.url = url;
	// 	// mret.onLineNum="";
	// 	ret[i] = mret;
	// }
	// curl_multi_handles(ret,3,TIMEOUT);

	// for(int i = 0; i < 10; ++i ) {
	// 	std::cout << reqArray[i].http_res<< endl;
	// }
	return 0 ;

}