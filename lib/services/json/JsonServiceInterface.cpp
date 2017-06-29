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
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>
#include <cmath>

#include "JsonServiceInterface.h"
#include "server/Log.h"
#include "database/TableTraverser.h"

using namespace std;

using namespace yucode::server;

namespace yucode {
namespace jsonservice {
	
// Service Interface
	void JsonServiceInterface::handleRequest(const Request& req, Reply& rep, const ServerContext & con) {
		RequestContext * requestContext = makeRequestContext(req);
		
		// Get general arguments for Json services
		if (!addRequestContext(req, rep, requestContext)) {
			responseErrorPackage(req, rep, requestContext, "service.fail", "Wrong syntax, required: 'method', expected: 'minified','query_id'");
		} else {
			// Try to dispatch service to its registered handler
			map<string, JsonServiceHandler>::iterator serviceHandlersIt = jsonServiceHandlers_.find(requestContext->method);
			if (serviceHandlersIt != jsonServiceHandlers_.end()) {
				// Let subclasses complete teh request
				if (addJsonRequestContext(req, rep, requestContext)) {
				
					// Dispatch service handler
					JsonServiceHandler serviceHandler = serviceHandlersIt->second;
					try {
						serviceHandler(req, rep, con, requestContext);
					} catch(...) {
						responseErrorPackage(req, rep, requestContext, requestContext->method, "Unexpected error");
					}
				}
			} else {
				responseErrorPackage(req, rep, requestContext, "service.fail", MKSTRING("Method '" << requestContext->method << "' not implemented"));
			}
		}
		if (requestContext)
			delete requestContext;
	}

// Preparing context from request
	RequestContext * JsonServiceInterface::makeRequestContext(const Request& req) {
		return new RequestContext();
	}
	
	bool JsonServiceInterface::addRequestContext(const Request& req, Reply& rep, RequestContext * requestContext) {
		bool status = true;
		
		unsigned long long queryId;
		if (req.loadArgumentVerifyUnsignedLongLong(queryId, "query_id"))
			requestContext->queryId = queryId;
		
		
		unsigned  int minified;
		if (req.loadArgumentVerifyUnsignedInt(minified, "minified"))
			requestContext->minified = (bool)minified;
		
		std::string method;
		if (req.loadArgumentVerifyString(method, "method"))
			requestContext->method = method;
		else
			status = false;
		
		return status;
	}

// Handling answer
	void JsonServiceInterface::responsePackage(Reply& rep, const Request& req, RequestContext * requestContext, yucode::jsonservice::Package & package) {
		stringstream ss;
		package.writeJson(ss, requestContext);
		string result = ss.str();
		rep.content = result;
		string resultSummary = result;
		if (result.size() > 160) {
			resultSummary = resultSummary.substr(0, 160);
			resultSummary += "...";
		}
		LOG("JsonServiceInterface::responsePackage: " << resultSummary);
		rep.headers.resize(req.origin.empty()? 2 : 4);
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = boost::lexical_cast<string>(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = "application/json";
		if (!req.origin.empty()) {
			rep.headers[2].name = "Access-Control-Allow-Origin";
			rep.headers[2].value = string("http://") + req.origin;
			rep.headers[3].name = "Access-Control-Allow-Credentials";
			rep.headers[3].value = "true";
		}
		rep.status = Reply::ok;
	}
}
}
