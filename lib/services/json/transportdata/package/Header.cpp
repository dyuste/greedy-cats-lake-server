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
#include "Header.h"

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "transportdata/model/ModelInterface.h"
#include "database/TableTraverser.h"
#include "server/Log.h"

using namespace std;

namespace yucode {
namespace jsonservice {

void Header::writeJson(std::ostream &os, bool minified) const {
	os << "{";
	
	string separator1 = "";
	for (ModelObjectListCollection::const_iterator it = modelObjectListCollection_.begin(); it != modelObjectListCollection_.end(); ++it) {
		os << separator1 << "\"" << it->first << "\": {";
		string separator2 = "";
		for (ModelObjectList::const_iterator itObj =  it->second.begin(); itObj != it->second.end(); ++itObj) {
			const shared_ptr<ModelInterface> & object = *itObj;
			os << separator2 << "\"" << object->getKey() << "\":";
			object->writeJson(os, minified);
			separator2 = ",";
		}
		os << "}";
		separator1 = ",";
	}
	
	os << "}";
}

}
}
