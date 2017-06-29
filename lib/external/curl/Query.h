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
#ifndef CURL_CURLINTERFACE_H_
#define CURL_CURLINTERFACE_H_

#include <string>
#include <map>
#include <sstream>

namespace curl {

	class Query {
	public:
		Query() : baseUrl_(std::string()) {}
		Query(const std::string & baseUrl) : baseUrl_(baseUrl) {}
		
		template<typename T>
		void addAttribute(const std::string & argName, const T& argValue) {
			std::stringstream ss;
			ss << argValue;
			attributes_[argName] = ss.str();
		}
		
		void setUrl(const std::string & baseUrl) { baseUrl_ = baseUrl; }
		
		std::string getUrl() const {
			std::string url = baseUrl_;
			std::map<std::string, std::string>::const_iterator attrIt = attributes_.begin();
			if (attrIt != attributes_.end()) {
				url += "?";
				url += attrIt->first + "=" + attrIt->second;
		
				for (++attrIt; attrIt != attributes_.end(); ++attrIt)
					url += "&" + attrIt->first + "=" + attrIt->second;
			}
			
			return url;
		}

	private:
		std::string baseUrl_;
		std::map<std::string, std::string> attributes_;
	};
}

#endif


