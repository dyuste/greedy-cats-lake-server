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

#include "RequestHandler.h"
#include <string>
#include "Server.h"
#include "Reply.h"
#include "Request.h"
#include "ServiceFileDispatch.h"
#include "Log.h"
#include <iostream>
#include <stdexcept>
#include <regex>
#include <vector>
#include <memory>

using namespace std;
namespace yucode {
namespace server {

void RequestHandler::handleRequest(Request& req, Reply& rep)
{
	bool dispatched = false;
#ifdef ACCESS_CONTROL_ENABLED	
	if (req.origin != "yucode.com" && req.origin != "es.yucode.com" && req.origin != "en.yucode.com") {
		LOG_ERROR_SECURITY("Access denied to origin: " << req.origin);
		rep = Reply::stock_Reply(Reply::internal_Server_error);
		return;
	}
#endif
	SetTimeStamp(req.timestamp);	
	try {
		vector<shared_ptr<ServiceInterface> > & services = server_->getServices();
		for (vector<shared_ptr<ServiceInterface> >::iterator it = services.begin(); 
		     it != services.end() && !dispatched; ++it) {
			shared_ptr<ServiceInterface> service = *it;
			if (service->matchRequest(req)) {
				service->handleRequest(req, rep, serverContext_);
				dispatched = true;
			}
		}
		
		if (!dispatched) {
			ServiceFileDispatch serviceFile;
			if (serviceFile.matchRequest(req)) {
				serviceFile.handleRequest(req, rep, serverContext_);
				dispatched = true;
			}
		}
	} catch (const char * e) {
		LOG_ERROR("Exception catched: " << e);
		rep = Reply::stock_Reply(Reply::internal_Server_error);
	} catch (std::exception & e) {
		LOG_ERROR("Exception catched: " << e.what());
		rep = Reply::stock_Reply(Reply::internal_Server_error);
	} catch (...) {
		LOG_ERROR("Exception catched: <unknown>");
		rep = Reply::stock_Reply(Reply::internal_Server_error);
	}
	
	if (!dispatched) {
		rep = Reply::stock_Reply(Reply::not_found);
		LOG_WARN("Request not found");
	}
	
	LOG_ELLAPSED_SINCE(req.timestamp, "service dispatched");
}


} // namespace server
} // namespace yucode
