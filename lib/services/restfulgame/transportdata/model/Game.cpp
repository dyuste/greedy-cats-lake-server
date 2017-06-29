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
#include "Game.h"
#include <iostream>

using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {
	
void Game::initDictionary() {
	addNumericField("id", "i", id_);
	addNumericField("sequence_num", "s", sequenceNum_);
	addNumericField("width", "w", width_);
	addNumericField("height", "h", height_);
	addNumericField("owner_user_id", "o", ownerUserId_);
	addNumericField("turn_player_id", "t", turnPlayerId_);
	addNumericField("random_game", "r", (randomGame_? 1 : 0));
	addNumericField("finished", "f", finished_);
	addNumericField("time_stamp", "ts", timeStamp_);
	addCsvObjectField("players", "p", players_);
	addCsvObjectField("cells", "c", cells_);
}

}
}
