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
#include "Server.h"
#include "Log.h"
#include <stdlib.h>
#include <stdio.h>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/exception/diagnostic_information.hpp> 
#include <vector>
#include <iostream>

#define DUMP_STACK_FILE "/server/core/yucode-server"

namespace yucode {
namespace server {

Server::Server(const std::string& address, const std::string& port,
		const std::string& doc_root, std::size_t thread_pool_size)
	: thread_pool_size_(thread_pool_size),
		signals_(io_service_), crash_signals_(io_service_), abort_signals_(io_service_),
		acceptor_(io_service_),
		new_Connection_(),
		ServerContext_(doc_root),
		RequestHandler_(ServerContext_, this),
		address_(address), port_(port)
{
	// Register to handle the signals that indicate when the Server should exit.
	// It is safe to register for the same signal multiple times in a program,
	// provided all registration for the specified signal is made through Asio.
	signals_.add(SIGINT);
	signals_.add(SIGTERM);
#if defined(SIGQUIT)
	signals_.add(SIGQUIT);
#endif // defined(SIGQUIT)
	signals_.async_wait(boost::bind(&Server::handle_stop, this));

	// core at crash
	crash_signals_.add(SIGSEGV);
	crash_signals_.add(SIGBUS);
	crash_signals_.async_wait(boost::bind(&Server::handle_crash, this));

	// core at abort
	abort_signals_.add(SIGABRT);
	abort_signals_.async_wait(boost::bind(&Server::handle_abort, this));
	
	setup_acceptor();
	start_accept();
}

void Server::run()
{
	// Create a pool of threads to run all of the io_services.
	std::vector<boost::shared_ptr<boost::thread> > threads;
	for (std::size_t i = 0; i < thread_pool_size_; ++i)
	{
		boost::shared_ptr<boost::thread> thread(new boost::thread(
					boost::bind(&boost::asio::io_service::run, &io_service_)));
		threads.push_back(thread);
	}

	// Wait for all threads in the pool to exit.
	for (std::size_t i = 0; i < threads.size(); ++i)
		threads[i]->join();
}

void Server::setup_acceptor()
{
	// Release any previous data
	boost::system::error_code ignore_error;
	acceptor_.cancel(ignore_error);	
	if (acceptor_.is_open()) {
		LOG("Server::setup_acceptor: release previous acceptor");
		acceptor_.close(ignore_error);
	}

	// Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
	LOG("Server::setup_acceptor: bind acceptor at " << address_ << ":" << port_);
	boost::asio::ip::tcp::resolver resolver(io_service_);
	boost::asio::ip::tcp::resolver::query query(address_, port_);
	boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
	acceptor_.open(endpoint.protocol());
	acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
	acceptor_.bind(endpoint);
	acceptor_.listen();
}

void Server::start_accept()
{
	new_Connection_.reset(new Connection(io_service_, RequestHandler_));
	acceptor_.async_accept(new_Connection_->socket(),
			boost::bind(&Server::handle_accept, this,
				boost::asio::placeholders::error));
}

void Server::handle_accept(const boost::system::error_code& e)
{
	try {
		if (!e){
			new_Connection_->start();
			start_accept();
		}
	} catch(...) {
		LOG_ERROR_CRITICAL("Server::handle_accept: Exception catched: " << boost::current_exception_diagnostic_information());
		setup_acceptor();
		start_accept();
		//io_service_.stop();
	}
}

void Server::handle_stop()
{
	LOG("Server shutdown");
	io_service_.stop();
}

void Server::handle_crash(){
	LOG_ERROR("FATAL: Application crashed");
	dumpstack();
	exit(-1);
}

void Server::handle_abort(){
	LOG_ERROR("FATAL: Application aborted");
	dumpstack();
	exit(-1);
}

void Server::dumpstack(void){
#ifdef DUMP_STACK_FILE
	char dbx[256];
	int timestamp = (int)time(NULL);
	int pid = getpid();
	sprintf(dbx, "gcore -o " DUMP_STACK_FILE ".%d.%d.dump %d ", pid, timestamp, pid);
	LOG_ERROR("running: " << dbx);
	system(dbx);
	LOG_ERROR("core dumped into " DUMP_STACK_FILE "." << pid << "." << timestamp << ".dump");
#endif
}

} // namespace server
} // namespace yucode
