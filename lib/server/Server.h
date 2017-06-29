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

#ifndef YUCODE_SERVER_SERVER_H
#define YUCODE_SERVER_SERVER_H

#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "Connection.h"
#include "RequestHandler.h"
#include "ServerContext.h"
#include "ServiceInterface.h"

namespace yucode {
namespace server {

class Server
	: private boost::noncopyable
{
public:
	/// Construct the Server to listen on the specified TCP address and port, and
	/// serve up files from the given directory.
	explicit Server(const std::string& address, const std::string& port,
			const std::string& doc_root, std::size_t thread_pool_size);

	/// Run the Server's io_service loop.
	void run();

	inline void addService(std::shared_ptr<ServiceInterface> service) { 
		services_.push_back(service);
		service->bootStrap();
	}
	
	inline std::vector<std::shared_ptr<ServiceInterface> > & getServices() { return services_; }
	
private:
	/// Configure and bind socket
	void setup_acceptor();

	/// Initiate an asynchronous accept operation.
	void start_accept();

	/// Handle completion of an asynchronous accept operation.
	void handle_accept(const boost::system::error_code& e);

	/// Handle a Request to stop the Server.
	void handle_stop();
	
	/// Crash/abort handling
	void handle_crash();
	void handle_abort();
	void dumpstack();

private:
	std::vector<std::shared_ptr<ServiceInterface> > services_;
	
	/// The number of threads that will call io_service::run().
	std::size_t thread_pool_size_;

	/// The io_service used to perform asynchronous operations.
	boost::asio::io_service io_service_;

	/// The signal_set is used to register for process termination notifications.
	boost::asio::signal_set signals_, crash_signals_, abort_signals_;

	/// Acceptor used to listen for incoming Connections.
	boost::asio::ip::tcp::acceptor acceptor_;

	/// The next Connection to be accepted.
	Connection_ptr new_Connection_;

	ServerContext ServerContext_;
	RequestHandler RequestHandler_;
	std::string address_;
	std::string port_;
};

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_SERVER_H
