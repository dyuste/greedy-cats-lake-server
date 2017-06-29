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
#include "Package.h"
#include <iostream>

namespace yucode {
namespace jsonservice {

void Package::writeJson(std::ostream &os, const RequestContext * requestContext) {
	bool minified = requestContext? requestContext->minified : false;
	os << "{\"header\":";
	header_.writeJson(os, minified);
	os << ",\"result\":";
	writeContentJson(os, minified);
	if (requestContext)
		os << ",\"query_id\":" << requestContext->queryId;
	os  << ",\"package_type\":\"" << packageType_ << "\""
	   << ",\"status\":" << (hasError()? "false" : "true")
	   << ",\"messages\":[\"" << errorMessage()<<"\"]}";
}

}
}
