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
#ifndef YUCODE_RESTFUL_GAMECONTROLLER_H
#define YUCODE_RESTFUL_GAMECONTROLLER_H

#include <string>
#include <boost/optional.hpp>
#include "services/json/transportdata/package/Header.h"
#include "services/json/RequestContext.h"
#include "transportdata/model/Game.h"
#include "transportdata/model/User.h"

namespace yucode {
namespace restfulgame {

class GameController {
public:
// Constants
	enum GameDefaults {
		DefaultGameWidth = 10,
		DefaultGameHeight = 10,
		MaxResourcesPerCell = 3,
		DefaultCellState = 1,
		MaxBasicThemes = 32
	};
	static const std::string TableGame;
	static const std::string TableUser;
	static const std::string TableVirtualUser;
	static const std::string TableGamePlayers;
	static const std::string TableGameCells;
	
public:
	static void bootStrap();
	
// Game Object
public:
	static bool userHasReadRightsOverGame(unsigned long long gameId, unsigned long long clientUserId);
	
	static boost::optional<Game> getGame(unsigned long long clientUserId, unsigned long long gameId, jsonservice::Header & header);
	static boost::optional<Game> createGame(const std::vector<unsigned long long> & userIds, unsigned long long clientUserId, const std::vector<std::string> & localUsers, bool random, jsonservice::Header & header);
	static boost::optional<Game> gameMove(unsigned long long gameId, unsigned int position, unsigned long long clientUserId, jsonservice::Header & header);
	static boost::optional<Game> createOrJoinRandomGame(unsigned long long clientUserId, jsonservice::Header & header);
	static std::vector<unsigned long long> getWaitingGames(unsigned long long clientUserId, jsonservice::Header & header);
	static std::vector<unsigned long long> getReadyGames(unsigned long long clientUserId, jsonservice::Header & header);
		static std::vector<unsigned long long> getFinishedGames(unsigned long long clientUserId, jsonservice::Header & header);
	static std::vector<unsigned long long> getUpdatesSinceTimeStamp(unsigned long long clientUserId, const std::map<unsigned long long, unsigned long long> & gameTimeStamps, jsonservice::Header & header);
	
protected:
	static boost::optional<Game> getGameBase(unsigned long long gameId, jsonservice::Header & header);
	static void addGamePlayers(unsigned long long clientUserId, Game & game, jsonservice::Header & header);
	static void addGameCells(Game & game, jsonservice::Header & header);

// User Object
public:
	static void createBomUser(unsigned long long userId);
	static void addUserToHeader(unsigned long long clientUserId, unsigned long long userId, jsonservice::Header & header);
	static std::vector<unsigned long long> searchUser(unsigned long long clientUserId, const std::string & pattern, jsonservice::Header & header);

protected:
	static std::vector<unsigned long long> getOrCreateVirtualUsersForUser(unsigned long long ownerUserId, const std::vector<std::string> & localUserNames);
};

}
}

#endif
