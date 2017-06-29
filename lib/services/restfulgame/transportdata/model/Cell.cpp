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
#include "Cell.h"
#include <iostream>

using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {

void Cell::initDictionary() {
	addNumericField("player_id", "p", userId_);
	addNumericField("resources", "r", resources_);
	addNumericField("state", "s", state_);
}

}
}
