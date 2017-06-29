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
#ifndef YUCODE_MODEL_MODEL_INTERFACE_H
#define YUCODE_MODEL_MODEL_INTERFACE_H

#include "server/Log.h"
#include "database/DataBase.h"
#include "misc/JsonObject.h"

namespace yucode {
namespace jsonservice {

class ModelInterface : public misc::JsonObject {
	friend class Header;
public:
	ModelInterface() : JsonObject(), key_(0) {}
	ModelInterface (long long key) : JsonObject(), key_(key) {}
	
	long long getKey() const { return key_; }
	void setKey(long long key) { key_ = key; }
	
private:
	long long key_;
};

}
}

#endif
