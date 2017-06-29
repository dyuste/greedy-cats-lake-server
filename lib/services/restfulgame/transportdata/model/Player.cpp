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
#include "Player.h"
#include <iostream>

using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {

void Player::initDictionary() {
	addNumericField("player_id", "p", playerId_);
	addNumericField("user_id", "u", userId_);
	addNumericField("virtual_user_id", "v", virtualUserId_);
	addNumericField("can_move", "c", canMove_);
	addNumericField("score", "s", score_);
}

}
}
