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

#ifndef YUCODE_SERVER_REPLY_H
#define YUCODE_SERVER_REPLY_H

#include <string>
#include <vector>
#include <boost/asio.hpp>
#include "Header.h"

namespace yucode {
namespace server {

/// A Reply to be sent to a client.
struct Reply
{
	/// The status of the Reply.
	enum status_type
	{
		ok = 200,
		created = 201,
		accepted = 202,
		no_content = 204,
		multiple_choices = 300,
		moved_permanently = 301,
		moved_temporarily = 302,
		not_modified = 304,
		bad_Request = 400,
		unauthorized = 401,
		forbidden = 403,
		not_found = 404,
		internal_Server_error = 500,
		not_implemented = 501,
		bad_gateway = 502,
		service_unavailable = 503
	} status;

	/// The headers to be included in the Reply.
	std::vector<Header> headers;

	/// The content to be sent in the Reply.
	std::string content;

	/// Convert the Reply into a vector of buffers. The buffers do not own the
	/// underlying memory blocks, therefore the Reply object must remain valid and
	/// not be changed until the write operation has completed.
	std::vector<boost::asio::const_buffer> to_buffers();

	/// Get a stock Reply.
	static Reply stock_Reply(status_type status);
};

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_REPLY_H