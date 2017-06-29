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
#ifndef YUCODE_SERVER_SERVERCONTEXT_H
#define YUCODE_SERVER_SERVERCONTEXT_H

#include <string>

namespace yucode {
namespace server {

struct ServerContext
{
	ServerContext(const std::string &doc_root)
		: doc_root_(doc_root) {}
		
	std::string doc_root_;
};

} // namespace server
} // namespace yucode

#endif