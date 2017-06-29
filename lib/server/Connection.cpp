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
#include "Connection.h"
#include <vector>
#include <boost/bind.hpp>
#include "RequestHandler.h"
#include "Log.h"

namespace yucode {
namespace server {

Connection::Connection(boost::asio::io_service& io_service,
		RequestHandler& handler)
	: strand_(io_service),
		socket_(io_service),
		RequestHandler_(handler)
{
}

boost::asio::ip::tcp::socket& Connection::socket()
{
	return socket_;
}

void Connection::start()
{
	static int req_sec_number = 0;
	Request_.seq_number = ++req_sec_number;
	Request_.remote_endpoint = socket_.remote_endpoint().address().to_string();
	
	socket_.async_read_some(boost::asio::buffer(buffer_),
			strand_.wrap(
				boost::bind(&Connection::handle_read, shared_from_this(),
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred)));
}

void Connection::handle_read(const boost::system::error_code& e,
		std::size_t bytes_transferred)
{
	if (!e)
	{
		boost::tribool result;
		boost::tie(result, boost::tuples::ignore) = RequestParser_.parse(
				Request_, buffer_.data(), buffer_.data() + bytes_transferred);

		if (result)
		{
			RequestHandler_.handleRequest(Request_, Reply_);
			boost::asio::async_write(socket_, Reply_.to_buffers(),
					strand_.wrap(
						boost::bind(&Connection::handle_write, shared_from_this(),
							boost::asio::placeholders::error)));
		}
		else if (!result)
		{
			Reply_ = Reply::stock_Reply(Reply::bad_Request);
			boost::asio::async_write(socket_, Reply_.to_buffers(),
					strand_.wrap(
						boost::bind(&Connection::handle_write, shared_from_this(),
							boost::asio::placeholders::error)));
		}
		else
		{
			boost::asio::ip::tcp::socket::non_blocking_io non_blocking_io(true);
			socket_.io_control(non_blocking_io);
			socket_.async_read_some(boost::asio::buffer(buffer_),
					strand_.wrap(
						boost::bind(&Connection::handle_read, shared_from_this(),
							boost::asio::placeholders::error,
							boost::asio::placeholders::bytes_transferred)));
		}
	}

	// If an error occurs then no new asynchronous operations are started. This
	// means that all shared_ptr references to the Connection object will
	// disappear and the object will be destroyed automatically after this
	// handler returns. The Connection class's destructor closes the socket.
}

void Connection::handle_write(const boost::system::error_code& e)
{
	if (!e)
	{
		// Initiate graceful Connection closure.
		boost::system::error_code ignored_ec;
		socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
	}

	// No new asynchronous operations are started. This means that all shared_ptr
	// references to the Connection object will disappear and the object will be
	// destroyed automatically after this handler returns. The Connection class's
	// destructor closes the socket.
}

} // namespace server
} // namespace yucode