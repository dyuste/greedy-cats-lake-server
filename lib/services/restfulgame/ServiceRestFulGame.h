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
#ifndef YUCODE_SERVER_SERVICE_RESTFUL_GAME_H
#define YUCODE_SERVER_SERVICE_RESTFUL_GAME_H

#include "services/restful/ServiceRestFul.h"
#include <string>
#include <vector>

namespace yucode {
namespace restfulgame {

	class ServiceRestFulGame : public restful::ServiceRestFul {
	public:
		ServiceRestFulGame();
		
	// Service Interfce
	public:
		bool matchRequest(const server::Request& req) const;
		void bootStrap();
	
	// Service Restful
	protected:
		void userDidSignUp(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void userWasUpdated(unsigned long long userId, jsonservice::Package & package);
		
	// Services
	protected:
		void serviceGameGet(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceGameGetUpdated(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceGameCreate(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceGameMove(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		void serviceGameRandom(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		
		void serviceUserSearch(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
		
		void serviceProfileGetSummary(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext);
	};
	
}
}

#endif