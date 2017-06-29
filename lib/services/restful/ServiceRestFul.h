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
#ifndef YUCODE_SERVER_SERVICE_RESTFUL_H
#define YUCODE_SERVER_SERVICE_RESTFUL_H

#include "services/json/JsonServiceInterface.h"
#include <string>
#include <vector>

namespace yucode {
namespace restful {
	
	class RestFulServiceData : public jsonservice::JsonServiceData {
	public:
		RestFulServiceData() 
		: JsonServiceData(), requiredSessionApi(true) {
		}
		
		RestFulServiceData(bool _requiredSessionApi) 
		: JsonServiceData(), requiredSessionApi(_requiredSessionApi) {
		}
		
		bool requiredSessionApi;
	};
	
	class ServiceRestFul : public jsonservice::JsonServiceInterface {
	public:
		ServiceRestFul();
		
	// Service Interfce
	public:
		void bootStrap();
		
		bool addJsonRequestContext(const server::Request& req, server::Reply& rep, jsonservice::RequestContext * requestContext);
	
	// Custom service registering
	protected:
		inline void registerRestFulServiceHandler(const std::string & method, const jsonservice::JsonServiceHandler &handler, bool requiredSessionApi) {
			registerJsonServiceHandler<RestFulServiceData>(method, handler, RestFulServiceData(requiredSessionApi));
		}
		
	// Overload to extend
	protected:
		virtual void userDidSignUp(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		}
		virtual void userWasUpdated(unsigned long long userId, jsonservice::Package & package) {
			
		}
		
	// Services
	private:
		void serviceSessionSignUp(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceSessionSignUpDummy(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceSessionSignIn(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceSessionLookUpUserName(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		
		void serviceSessionExtendAccount(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceSessionExtendFacebook(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		
		void serviceSessionRegisterTokenIos(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
	};
	
}
}

#endif