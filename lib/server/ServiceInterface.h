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
#ifndef YUCODE_SERVER_SERVICE_INTERFACE_H
#define YUCODE_SERVER_SERVICE_INTERFACE_H

#include <string>
#include "Request.h"
#include "Reply.h"
#include "ServerContext.h"

namespace yucode {
namespace server {

	class ServiceInterface {
	public:
		/// Returns whether this service applies to req.
		virtual bool matchRequest(const Request& req) const = 0;
		
		/// Handle a Request and produce a Reply.
		virtual void handleRequest(const Request& req, Reply& rep, const ServerContext & con) = 0;
		
		virtual void bootStrap() {
		}
	};
}
}

#endif