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
#include "ServiceFileDispatch.h"

#include <string>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include "mime_types.h"

namespace yucode {
namespace server {

	
	bool ServiceFileDispatch::matchRequest(const Request& req) const {
		return true;
	}
	
	/// Handle a Request and produce a Reply.
	void ServiceFileDispatch::handleRequest(const Request& req, Reply& rep, const ServerContext & con) {
		std::string Request_path = req.decoded_uri;
		
		// Request path must be absolute and not contain "..".
		if (req.decoded_uri.empty() || req.decoded_uri[0] != '/'
				|| req.decoded_uri.find("..") != std::string::npos)
		{
			rep = Reply::stock_Reply(Reply::bad_Request);
			return;
		}
		
		// If path ends in slash (i.e. is a directory) then add "index.html".
		if (req.decoded_uri[Request_path.size() - 1] == '/')
		{
			Request_path += "index.html";
		}

		std::string extension = req.extension;
		if (extension.empty())
			extension = "html";
		
		
		// Open the file to send back.
		std::string full_path = con.doc_root_ + Request_path;
		if (boost::filesystem::is_directory(boost::filesystem::path(full_path))) {
			full_path += "/index.html";
		}
		std::ifstream is(full_path.c_str(), std::ios::in | std::ios::binary);
		if (!is)
		{
					rep = Reply::stock_Reply(Reply::not_found);
					return;
		}

		// Fill out the Reply to be sent to the client.
		rep.status = Reply::ok;
		char buf[512];
		while (is.read(buf, sizeof(buf)).gcount() > 0)
			rep.content.append(buf, is.gcount());
		rep.headers.resize(2);
		rep.headers[0].name = "Content-Length";
		rep.headers[0].value = boost::lexical_cast<std::string>(rep.content.size());
		rep.headers[1].name = "Content-Type";
		rep.headers[1].value = mime_types::extension_to_type(extension);
	}
		
}
}
