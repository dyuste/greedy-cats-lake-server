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

#ifndef YUCODE_SERVER_REQUEST_HANDLER_H
#define YUCODE_SERVER_REQUEST_HANDLER_H

#include <string>
#include <boost/noncopyable.hpp>
#include "ServerContext.h"

namespace yucode {
namespace server {

class Server;

struct Reply;
struct Request;

/// The common handler for all incoming Requests.
class RequestHandler
	: private boost::noncopyable
{
public:
	explicit RequestHandler(const ServerContext& serverContext, Server* server)
		: serverContext_(serverContext), server_(server) {}

	void handleRequest(Request& req, Reply& rep);

private:
	const ServerContext &serverContext_;
	Server * server_;
	
	static bool url_decode(const std::string& in, std::string& out);
	
};

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_REQUEST_HANDLER_H