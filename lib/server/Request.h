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

#ifndef YUCODE_SERVER_REQUEST_H
#define YUCODE_SERVER_REQUEST_H

#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include "Header.h"
#include "Log.h"
#include "misc/Utilities.h"
#include "database/DataBase.h"

namespace yucode {
namespace server {

/// A Request received from a client.
struct Request
{
	long long seq_number;
	TimeStamp timestamp;
	std::string remote_endpoint;
	std::string method;
	std::string uri;
	std::string referer;
	std::string origin;
	int http_version_major;
	int http_version_minor;
	std::vector<Header> headers;
	std::string extension;
	std::string root_folder;
	std::string decoded_uri;
	std::map<std::string, std::string> arguments;
	std::map<std::string, std::string> cookies;
	
	inline bool loadArgument(std::string & to, std::string argName, bool paranoiacSQLInjection = true) const {
		std::map<std::string, std::string>::const_iterator argIt = arguments.find(argName);
		if (argIt == arguments.end()) 
			return false;
		to = argIt->second;
		return true;
	}
		
	bool loadArgumentVerifyString(std::string & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		if (tmp.find(';')!=std::string::npos 
			|| tmp.find('\'')!=std::string::npos 
			|| tmp.find('\\')!=std::string::npos) {
			LOG_ERROR_SECURITY("SQL Injection alert at argument " << argName << ": " << tmp);
			return false;
		}
		
		to = tmp;
		return true;
	}
	
	bool loadArgumentVerifyStringText(std::string & to, std::string argName) const {
		std::string tmp;
		if (!loadArgumentVerifyString(tmp, argName))
			return false;
		to = DataBase::singleton().escapeString(tmp);
		return true;
	}
	
	bool loadArgumentVerifyDouble(double & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtod(tmp.c_str(), &endptr);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		return true;
	}
	
	bool loadArgumentVerifyLong(long int & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtol(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		return true;
	}
	
	bool loadArgumentVerifyUnsignedInt(unsigned int & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		unsigned long realval = strtoul(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		to = (unsigned int)realval;
		return true;
	}
		
	bool loadArgumentVerifyTimeT(time_t & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		unsigned long realval = strtoul(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		to = (time_t)realval;
		return true;
	}
	
	bool loadArgumentVerifyUnsignedLong(unsigned long int & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtoul(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		return true;
	}
	
	bool loadArgumentVerifyLongLong(long long & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtoll(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		return true;
	}
	
	bool loadArgumentVerifyUnsignedLongLong(unsigned long long & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtoull(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
			return false;
		}
		return true;
	}
	
	bool loadArgumentVerifyCsvLongLong(std::vector<long long> & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		std::vector<std::string> tokens;
		&misc::Utilities::split(tmp, ',', tokens);
		for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			char *endptr = 0;
			long long val = strtoll(it->c_str(), &endptr, 10);
			if (endptr == tmp.c_str()) {
				LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
				return false;
			}
			to.push_back(val);
		}

		return true;
	}
	
	bool loadArgumentVerifyCsvUnsignedLongLong(std::vector<unsigned long long> & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
	
		std::vector<std::string> tokens;
		&misc::Utilities::split(tmp, ',', tokens);
		for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it) {
			char *endptr = 0;
			unsigned long long val = strtoull(it->c_str(), &endptr, 10);
			if (endptr == tmp.c_str()) {
				LOG_ERROR_SECURITY("invalid input argument " << argName << " " << tmp);
				return false;
			}
			to.push_back(val);
		}

		return true;
	}
	bool loadArgumentVerifyCsvStringText(std::vector<std::string> & to, std::string argName) const {
		std::string tmp;
		if (!loadArgument(tmp, argName))
			return false;
		
		std::vector<std::string> tokens;
		&misc::Utilities::split(tmp, ',', tokens);
		for (std::vector<std::string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
			*it = DataBase::singleton().escapeString(*it);
		to = tokens;
		return true;
	}
	
	inline bool loadCookie(std::string & to, std::string argName) const {
		std::map<std::string, std::string>::const_iterator argIt = cookies.find(argName);
		if (argIt == cookies.end()) 
			return false;
		to = argIt->second;
		return true;
	}
	
	bool loadCookieVerifyLongLong(long long & to, std::string argName) const {
		std::string tmp;
		if (!loadCookie(tmp, argName))
			return false;
		
		char *endptr = 0;
		to = strtoll(tmp.c_str(), &endptr, 10);
		if (endptr == tmp.c_str()) {
			LOG_ERROR_SECURITY("invalid input cookie " << argName << " " << tmp);
			return false;
		}
		return true;
	}
};

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_REQUEST_H
