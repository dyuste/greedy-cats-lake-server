/*
 *  Copyright (c) 2015 David Yuste Romero
 *
 *  THIS MATERIAL IS PROVIDED AS IS, WITH ABSOLUTELY NO WARRANTY EXPRESSED
 *  OR IMPLIED.  ANY USE IS AT YOUR OWN RISK.
 *
 *  Permission is hereby granted to use or copy this program
 *  for any purpose,  provided the above notices are retained on all copies.
 *  Permission to modify the code and to distribute modified code is granted,
 *  provided the above notices are retained, and a notice that the code was
 *  modified is included with the above copyright notice.
 */
#ifndef CURL_CURLDOWNLOADER_H_
#define CURL_CURLDOWNLOADER_H_

#include <curl/curl.h>
#include <curl/easy.h>
#include <string>
#include <ostream>
#include "Query.h"

namespace curl {
	
	static size_t CurlInterfaceWriteCallback(void *contents, size_t size, size_t nmemb, void *strPtr)
	{ 
		std::string * str = static_cast<std::string*>(strPtr);
		size_t realsize = size * nmemb;
		str->append(static_cast<const char*>(contents), realsize);
		return realsize;
	}
	
	class Downloader {
	public:
		static bool Download(const std::string & url, std::string * outputString) {
			CURL *curl;
			CURLcode res;
			long int written;

			curl = curl_easy_init();
			if (!curl)
				return false;

			outputString->clear();
			curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlInterfaceWriteCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputString);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0); 
			res = curl_easy_perform(curl);
			
			curl_easy_cleanup(curl);
			
			return !outputString->empty();
		}
		
		static bool Download(const Query & query, std::string * outputString) {
			return Download(query.getUrl(), outputString);
		}
	};

}

#endif


