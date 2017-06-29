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
#ifndef YUCODE_RESTFUL_RESTFULCONTROLLER_H
#define YUCODE_RESTFUL_RESTFULCONTROLLER_H

#include <string>
#include <vector>
#include <boost/optional.hpp>
#include "server/Request.h"

namespace yucode {
namespace restful {

class RestFulController {
public:
	static const std::string TableRestFulUser;
	static const std::string TableRestFulSession;
	static const std::string TableRestFulFacebookUser;
	static const std::string Md5SaltToken;
	
public:
	static void bootStrap();
	
	static bool availableUserName(const std::string & userName);
	static bool availableEmail(const std::string & email);
	static bool isDummyUser(unsigned long long userId);
	
	static std::string makeDummyUserName();
	static std::string makeDummyPassMd5();
		
	static boost::optional<std::string> sessionSignUp(const std::string & userName, const std::string & passMd5, const std::string & email, const std::string & name, const server::Request& req);
	static boost::optional<std::string> sessionSignIn(const std::string & userName, const std::string & passMd5, const server::Request& req);
	static boost::optional<unsigned long long> getAuthenticatedUser(const std::string & sessionKey, const server::Request& req);
	
	static bool sessionExtendAccount(unsigned long long userId, const std::string & userName, const std::string & passMd5, const std::string & email, const std::string & name, const server::Request& req);
	static void sessionExtendFacebook(unsigned long long userId, const std::string & fbToken);
	
};

}
}

#endif
