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

#ifndef YUCODE_SERVER_CONNECTION_H
#define YUCODE_SERVER_CONNECTION_H

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "Reply.h"
#include "Request.h"
#include "RequestHandler.h"
#include "RequestParser.h"

namespace yucode {
namespace server {

/// Represents a single Connection from a client.
class Connection
	: public boost::enable_shared_from_this<Connection>,
		private boost::noncopyable
{
public:
	/// Construct a Connection with the given io_service.
	explicit Connection(boost::asio::io_service& io_service,
			RequestHandler& handler);

	/// Get the socket associated with the Connection.
	boost::asio::ip::tcp::socket& socket();

	/// Start the first asynchronous operation for the Connection.
	void start();

private:
	/// Handle completion of a read operation.
	void handle_read(const boost::system::error_code& e,
			std::size_t bytes_transferred);

	/// Handle completion of a write operation.
	void handle_write(const boost::system::error_code& e);

	/// Strand to ensure the Connection's handlers are not called concurrently.
	boost::asio::io_service::strand strand_;

	/// Socket for the Connection.
	boost::asio::ip::tcp::socket socket_;

	/// The handler used to process the incoming Request.
	RequestHandler& RequestHandler_;

	/// Buffer for incoming data.
	boost::array<char, 8192> buffer_;

	/// The incoming Request.
	Request Request_;

	/// The parser for the incoming Request.
	RequestParser RequestParser_;

	/// The Reply to be sent back to the client.
	Reply Reply_;
};

typedef boost::shared_ptr<Connection> Connection_ptr;

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_CONNECTION_H