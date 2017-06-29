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
#include "ServiceRestFul.h"

#include <string>
#include <sstream>
#include <boost/bind.hpp>
#include <cmath>

#include "database/TableTraverser.h"
#include "server/Log.h"
#include "misc/Utilities.h"
#include "notifications/NotificationController.h"
#include "controller/RestFulController.h"

using namespace std;
using namespace yucode::server;
using namespace yucode::jsonservice;

namespace yucode {
namespace restful {

	ServiceRestFul::ServiceRestFul() {
		registerRestFulServiceHandler("session.signup", /* user_name:String,pass_md5:String,name:String,email:String */
				    boost::bind(&ServiceRestFul::serviceSessionSignUp, this, _1, _2, _3, _4), false);
		registerRestFulServiceHandler("session.signup.dummy", /* -> user_id:Integer,pass_md5:String,session_key:String */
				    boost::bind(&ServiceRestFul::serviceSessionSignUpDummy, this, _1, _2, _3, _4), false);
		registerRestFulServiceHandler("session.extend.facebook", /* fb_token:String */
				    boost::bind(&ServiceRestFul::serviceSessionExtendFacebook, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("session.extend.account", /* user_name:String,pass_md5:String,name:String,email:String */
				    boost::bind(&ServiceRestFul::serviceSessionExtendAccount, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("session.signin", /* user_name:String,pass_md5:String */
				    boost::bind(&ServiceRestFul::serviceSessionSignIn, this, _1, _2, _3, _4), false);
		registerRestFulServiceHandler("session.lookup.username", /* user_name:String*/
				    boost::bind(&ServiceRestFul::serviceSessionLookUpUserName, this, _1, _2, _3, _4), false);
		registerRestFulServiceHandler("session.register.token.ios", /* token:String*/
				    boost::bind(&ServiceRestFul::serviceSessionRegisterTokenIos, this, _1, _2, _3, _4), true);
	}
	
// Service Interfce
	void ServiceRestFul::bootStrap() {
		RestFulController::bootStrap();
	}

// Extend services with session
	bool ServiceRestFul::addJsonRequestContext(const Request& req, Reply& rep, RequestContext * requestContext) {
		
		// session key check
		shared_ptr<RestFulServiceData> serviceData 
			= dynamic_pointer_cast<RestFulServiceData> (getServiceData(requestContext->method));
		bool requiredSessionApi = true;
		if (serviceData) {
			requiredSessionApi = serviceData->requiredSessionApi;
		} else {
			LOG_ERROR("JsonServiceInterface::addRequestContext: Unexpected missing RestFulServiceData for " << requestContext->method);
		}
		
		if (requiredSessionApi) {
			std::string sessionKey;
			if (!req.loadArgumentVerifyStringText(sessionKey, "session_key")) {
				LOG_WARN("ServiceRestFul::addRequestContext: Missing session_key argument for method " << requestContext->method);
				responseErrorPackage(req, rep, requestContext, "session.failed.key", "Missing or wrong session key, required: 'session_key'");
				return false;
			}
			
			boost::optional<unsigned long long> userId = RestFulController::getAuthenticatedUser(sessionKey, req);
			if (!userId) {
				responseErrorPackage(req, rep, requestContext, "session.failed.key", "Missing or wrong session key, required: 'session_key'");
				return false;
			}
			requestContext->userId = *userId;
		}
		return true;
	}
	
// Session Services
	void ServiceRestFul::serviceSessionSignUp(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string userName, passMd5, email, name;
		if (!req.loadArgumentVerifyStringText(userName, "user_name")
			|| !req.loadArgumentVerifyStringText(passMd5, "pass_md5")
			|| !req.loadArgumentVerifyStringText(email, "email")) {
			responseErrorPackage(req, rep, requestContext, "session.signup", "Missing or wrong argument: required 'user_name','pass_md5','email'");
			return;
		}
		req.loadArgumentVerifyStringText(name, "name");
		
		// Perform sign up + sign in
		boost::optional<string> sessionKey = RestFulController::sessionSignUp(userName, passMd5, email, name, req);
		if (!sessionKey) {
			responseErrorPackage(req, rep, requestContext, "session.signup", MKSTRING("Sign up failed (ST1)"));
			return;
		}
		
		boost::optional<unsigned long long> userId = RestFulController::getAuthenticatedUser(*sessionKey, req);
		if (!userId) {
			LOG_ERROR("ServiceRestFul::serviceSessionSignUp: Failed to authenticate just created user: " << userName << "-" << passMd5);
			responseErrorPackage(req, rep, requestContext, "session.signup", "Sign up failed (ST2)");
			return;
		}
		
		requestContext->userId = *userId;
		
		// Let application finish sign up details
		userDidSignUp(req, rep, serverContext, requestContext);
		
		// Finalize and send
		DefaultPackage package("session.signup");
		userWasUpdated(*userId, package);
		package.addNumericField("user_id", "u", *userId);
		package.addLiteralField("session_key", "s", *sessionKey);
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFul::serviceSessionSignUpDummy(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string userName = RestFulController::makeDummyUserName();
		string passMd5 = RestFulController::makeDummyPassMd5();
		
		// Perform sign up + sign in
		boost::optional<string> sessionKey = RestFulController::sessionSignUp(userName, passMd5, "", "", req);
		if (!sessionKey) {
			responseErrorPackage(req, rep, requestContext, "session.signup.dummy", MKSTRING("Sign up dummy failed (ST1)"));
			return;
		}
		
		boost::optional<unsigned long long> userId = RestFulController::getAuthenticatedUser(*sessionKey, req);
		if (!userId) {
			LOG_ERROR("ServiceRestFul::serviceSessionSignUpDummy: Failed to authenticate just created user: " << userName << "-" << passMd5);
			responseErrorPackage(req, rep, requestContext, "session.signup.dummy", "Sign up dummy failed (ST2)");
			return;
		}
		
		requestContext->userId = *userId;
		
		// Let application finish sign up details
		userDidSignUp(req, rep, serverContext, requestContext);
		
		// Finalize and send
		DefaultPackage package("session.signup.dummy");
		userWasUpdated(*userId, package);
		package.addNumericField("user_id", "u", *userId);
		package.addLiteralField("user_name", "n", userName);
		package.addLiteralField("pass_md5", "p", passMd5);
		package.addLiteralField("session_key", "s", *sessionKey);
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFul::serviceSessionExtendFacebook(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string fbToken;
		if (!req.loadArgumentVerifyStringText(fbToken, "fb_token")) {
			responseErrorPackage(req, rep, requestContext, "session.extend.facebook", "Missing or wrong argument: required 'fb_token'");
			return;
		}
		
		RestFulController::sessionExtendFacebook(requestContext->userId, fbToken);
		
		// Finalize and send
		DefaultPackage package("session.extend.facebook");
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFul::serviceSessionExtendAccount(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string userName, passMd5, email, name;
		if (!req.loadArgumentVerifyStringText(userName, "user_name")
			|| !req.loadArgumentVerifyStringText(passMd5, "pass_md5")
			|| !req.loadArgumentVerifyStringText(email, "email")) {
			responseErrorPackage(req, rep, requestContext, "session.extend.account", "Missing or wrong argument: required 'user_name','pass_md5','email'");
			return;
		}
		req.loadArgumentVerifyStringText(name, "name");
		
		// Perform sign up + sign in
		if (!RestFulController::sessionExtendAccount(requestContext->userId, userName, passMd5, email, name, req)) {
			responseErrorPackage(req, rep, requestContext, "session.extend.account", MKSTRING("Account extension failed"));
			return;
		}
		
		// Finalize and send
		DefaultPackage package("session.extend.account");
		userWasUpdated(requestContext->userId, package);
		responsePackage(rep, req, requestContext, package);
	}
		
	void ServiceRestFul::serviceSessionSignIn(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string userName, passMd5;
		if (!req.loadArgumentVerifyStringText(userName, "user_name")
			|| !req.loadArgumentVerifyStringText(passMd5, "pass_md5")) {
			responseErrorPackage(req, rep, requestContext, "session.signin", "Missing or wrong argument: required 'user_name','pass_md5'");
			return;
		}
		
		// Perform sign up + sign in
		boost::optional<string> sessionKey = RestFulController::sessionSignIn(userName, passMd5, req);
		if (!sessionKey) {
			responseErrorPackage(req, rep, requestContext, "session.signin", MKSTRING("Sign in failed (ST1)"));
			return;
		}
		
		boost::optional<unsigned long long> userId = RestFulController::getAuthenticatedUser(*sessionKey, req);
		if (!userId) {
			LOG_ERROR("ServiceRestFul::serviceSessionSignUp: Failed to authenticate just created user: " << userName << "-" << passMd5);
			responseErrorPackage(req, rep, requestContext, "session.signin", "Sign in failed (ST2)");
			return;
		}
		
		requestContext->userId = *userId;
	
		bool dummy = RestFulController::isDummyUser(*userId);
	
		// Finalize and send
		DefaultPackage package("session.signin");
		userWasUpdated(*userId, package);
		package.addNumericField("user_id", "u", *userId);
		package.addNumericField("dummy", "d", *userId);
		package.addLiteralField("session_key", "s", *sessionKey);
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFul::serviceSessionLookUpUserName(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string userName;
		if (!req.loadArgumentVerifyStringText(userName, "user_name")) {
			responseErrorPackage(req, rep, requestContext, "session.lookup.username", "Missing or wrong argument: required 'user_name'");
			return;
		}
		
		// Perform sign up + sign in
		bool available = RestFulController::availableUserName(userName);
		
		// Finalize and send
		DefaultPackage package("session.lookup.username");
		package.addLiteralField("user_name", "u", userName);
		package.addNumericField("available", "a", (int)(available? 1 : 0));
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFul::serviceSessionRegisterTokenIos(const Request& req, Reply& rep, const ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		// Fetch and verify arguments
		string token;
		if (!req.loadArgumentVerifyStringText(token, "token")) {
			responseErrorPackage(req, rep, requestContext, "session.register.token.ios", "Missing or wrong argument: required 'token'");
			return;
		}

		notifications::NotificationController::singleton().addUserToken(requestContext->userId, 
			notifications::NotificationTokenType::NotificationIosToken, token);
		
		// Finalize and send
		DefaultPackage package("session.register.token.ios");
		responsePackage(rep, req, requestContext, package);
	}
}
}
