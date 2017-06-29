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
#include "GameBotsDaemon.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include "database/TableTraverser.h"
#include "server/Log.h"
#include "misc/Utilities.h"
#include "GameController.h"
#include "GameAlgorithm.h"
#include "controller/GameController.h"
#include "notifications/NotificationController.h"
#include "services/json/transportdata/package/Package.h"
#include "services/restful/controller/RestFulController.h"

using namespace std;
using namespace yucode::restful;

namespace yucode {
namespace restfulgame {

const std::string GameBotsDaemon::TableDaemons = "bot_game";

// BootStrap
void GameBotsDaemon::bootStrap() {
	bootStrapDataBase();
	bootStrapLoadBots();
}

void GameBotsDaemon::bootStrapDataBase() {
	DataBase::singleton().createTableIfNotExists(TableDaemons,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
		"user_id BIGINT UNSIGNED NOT NULL,"
		"KEY user_id(user_id)");

#define BOT(USERNAME, NAME) createBot(USERNAME, NAME);
#include "Bots.def"
#undef BOT	
}

void GameBotsDaemon::createBot(const string & userName, const string & name) {
	string passMd5 = RestFulController::makeDummyPassMd5();
	server::Request req;
	try {
	boost::optional<string> sessionKey = RestFulController::sessionSignUp(userName, passMd5, MKSTRING(userName << "@yucode.com"), name, req);
	if (sessionKey) {
		boost::optional<unsigned long long> userId = RestFulController::getAuthenticatedUser(*sessionKey, req);
		if (userId) {
			GameController::createBomUser(*userId);
			DataBase::singleton().executeInsert(TableDaemons,
				"user_id",
				MKSTRING("("<<*userId<<")"));
			LOG("GameBotsDaemon::createBot: Created bot " << userName << " (" << name << ") with id " << *userId);
		}
	}
	} catch(...) {
	}
}

void GameBotsDaemon::bootStrapLoadBots() {
	TableTraverser botTraverser(
		MKSTRING("SELECT user_id FROM " << TableDaemons));
	for (TableTraverser::iterator botIt = botTraverser.begin(); botIt != botTraverser.end(); ++ botIt)
		bots_.push_back(botIt.rowAsUnsignedLongLong(0));
}

// Daemon services
void GameBotsDaemon::runRandomGamesJoiner() {
	LOG("GameBotsDaemon::runRandomGamesJoiner: Started random games joiner bot");
	boost::posix_time::seconds sleepTime(20);
	
	while (1) {
		joinRandomGames();
		boost::this_thread::sleep(sleepTime);
	}
}

void GameBotsDaemon::runBotTurnPlayer() {
	boost::posix_time::seconds sleepTime(6);
	
	LOG("GameBotsDaemon::runBotTurnPlayer: Started turn player bot");
	while (1) {
		vector<unsigned long long> gameIds = getBotTurnGames();
		if (gameIds.size() > 0) {
			LOG("GameBotsDaemon::runBotTurnPlayer: " << gameIds.size() << " bot turn games");
			for (vector<unsigned long long>::const_iterator it = gameIds.begin(); it != gameIds.end(); ++it)
				playBotTurn(*it);
		}
		
		boost::this_thread::sleep(sleepTime);
	}
}

// Bot joins game
void GameBotsDaemon::joinRandomGames() {
	vector<unsigned long long> joinedGameIds;
	int joinedGames = 0;
	while (joinedGames < 100) {
		unsigned long long botUserId = getAvailableBotId();
		if (botUserId > 0) {	
			// Try to insert user into an already existing random game
			DataBase::singleton().executeQuery(MKSTRING(
				"INSERT INTO " << GameController::TableGamePlayers << "(player_id, game_id, user_id) "
				"SELECT COUNT(p.player_id),g.id," << botUserId  << " "
				"FROM " << GameController::TableGame << " g "
				"INNER JOIN " << GameController::TableGamePlayers << " p "
					"ON p.game_id=g.id "
				"WHERE g.random_game=1 "
					"AND NOT EXISTS(SELECT 1 FROM " << GameController::TableGamePlayers << " p2 "
					" WHERE p2.user_id="<< botUserId <<" AND p2.game_id=g.id) "
				"GROUP BY g.id "
				"HAVING COUNT(p.player_id) < 4 "
				"LIMIT 1"	
			));
		
			if (DataBase::singleton().getAffectedRows() > 0) {
				// In case of success, place the user in a cell and return the game
				unsigned long long playerAbsId = DataBase::singleton().getLastInsertedId();
			
				TableTraverser gameTraverser(
					MKSTRING("SELECT p.game_id,g.time_stamp,p.player_id FROM " << GameController::TableGamePlayers << " p "
						"INNER JOIN " << GameController::TableGame << " g ON g.id=p.game_id "
						"WHERE p.id=" << playerAbsId));

				TableTraverser::iterator gameIt = gameTraverser.begin();
				if (gameIt != gameTraverser.end()) {
					unsigned long long gameId = gameIt.rowAsUnsignedLongLong(0);
					unsigned long long timeStamp = gameIt.rowAsUnsignedLongLong(1);
					unsigned long long playerId = gameIt.rowAsUnsignedLongLong(2);

					joinedGameIds.push_back(gameId);
	
					string cellConstraint = MKSTRING(
						"(position-floor(position/"<< GameController::DefaultGameWidth <<")*"<< GameController::DefaultGameWidth <<")>2 "
						"AND floor(position/"<< GameController::DefaultGameWidth <<")>2 "
						"AND (position-floor(position/"<< GameController::DefaultGameWidth <<")*"<< GameController::DefaultGameWidth <<")<("<< GameController::DefaultGameWidth <<"-2) "
						"AND floor(position/"<< GameController::DefaultGameWidth <<")<("<< GameController::DefaultGameWidth <<"-2)");
					DataBase::singleton().executeQuery(MKSTRING(
						"UPDATE " << GameController::TableGameCells << " "
						"SET player_id=" << playerId << " "
						"WHERE game_id= " << gameId << " AND player_id IS NULL and state>0 "
						" AND (" << cellConstraint << ")"
						"ORDER BY RAND() "
						"LIMIT 1"));
					
					DataBase::singleton().executeQuery(MKSTRING(
						"UPDATE " << GameController::TableGamePlayers << " p "
						"INNER JOIN " << GameController::TableGameCells << " c "
						"SET p.score=p.score+c.resources "
						"WHERE p.id= " << playerAbsId << " AND c.player_id=" << playerId <<
						" AND c.game_id=" << gameId));

					DataBase::singleton().executeQuery(MKSTRING(
						"UPDATE " << GameController::TableGame << " "
						"SET time_stamp=now() "
						"WHERE id= " << gameId << " "));		
					
					TableTraverser gameTraverser(
						MKSTRING("SELECT time_stamp FROM " << GameController::TableGame << " "
							"WHERE id=" << gameId));
	
					TableTraverser::iterator gameIt = gameTraverser.begin();
					if (gameIt != gameTraverser.end())
						timeStamp = gameIt.rowAsUnsignedLongLong(0);

					// Notify the group
					notifications::NotificationData payLoad;
					payLoad.addLiteralField("package_type", "p", "notification.game");
					payLoad.addNumericField("game_id", "g", gameId);
					payLoad.addNumericField("time_stamp", "ts", timeStamp);
					notifications::NotificationController::singleton().pushGroupNotification(
					notifications::NotificationGroupTypeGame, gameIt.rowAsUnsignedLongLong(0), payLoad);
				}
				++joinedGames;
			} else {
				break;
			}
		}
	}
	
	LOG("GameBotsDaemon::joinRandomGames: joined " << joinedGames << " games");
	if (joinedGames >= 100) {
		LOG_WARN("GameBotsDaemon::joinRandomGames : Possible load excess, more than 100 queued games");
	}
}

unsigned long long GameBotsDaemon::getAvailableBotId() {
	return bots_[rand() % bots_.size()];
}

// Bot plays
vector<unsigned long long> GameBotsDaemon::getBotTurnGames() {
	vector<unsigned long long> gameIds;
	
	TableTraverser gameTraverser(
		MKSTRING("SELECT g.id FROM " << TableDaemons << " d "
			"INNER JOIN " << GameController::TableGamePlayers << " p "
				"ON p.user_id=d.user_id "
			"INNER JOIN " << GameController::TableGame << " g "
				"ON g.id=p.game_id "
			"WHERE g.turn_player_id=p.player_id AND g.finished=0 "
				"AND g.time_stamp < DATE_SUB(NOW(),INTERVAL 5 SECOND)"
		));
	int limit = 0;
	for (TableTraverser::iterator it = gameTraverser.begin(); it != gameTraverser.end() && limit < 100; ++it, ++limit)
		gameIds.push_back(it.rowAsUnsignedLongLong(0));
	
	if (limit >= 100) {
		LOG_WARN("GameBotsDaemon::getBotTurnGames : Possible load excess, more than 100 queued games");
	}
	
	return gameIds;
}

void GameBotsDaemon::playBotTurn(unsigned long long gameId) {
	// Fetch game info
	TableTraverser gameTraverser(
		MKSTRING("SELECT g.width,g.height,g.turn_player_id,c.position,p.user_id FROM " << GameController::TableGame << " g "
			"INNER JOIN " << GameController::TableGameCells << " c "
				"ON c.game_id=g.id AND c.player_id=g.turn_player_id "
			"INNER JOIN " << GameController::TableGamePlayers << " p "
				"ON p.game_id=g.id AND p.player_id=g.turn_player_id "
			"WHERE g.id=" << gameId
		));
	TableTraverser::iterator gameIt = gameTraverser.begin();
	if (gameIt == gameTraverser.end() || gameIt.isNull(5)) {
		LOG_ERROR("GameBotsDaemon::playBotTurn : failed to fetch waiting for bot game " << gameId);
		return;
	}
	
	unsigned int width = gameIt.rowAsUnsignedInt(0);
	unsigned int height = gameIt.rowAsUnsignedInt(1);
	unsigned int playerId = gameIt.rowAsUnsignedInt(2);
	unsigned int position = gameIt.rowAsUnsignedInt(3);
	unsigned int userId = gameIt.rowAsUnsignedLongLong(4);
	
	// Run AI
	server::TimeStamp ts;
	SetTimeStamp(ts);
	unsigned int nextPosition = GameAlgorithm::minMaxGame(gameId, width, height, playerId);
	LOG_ELLAPSED_SINCE(ts, "GameBotsDaemon::playBotTurn : optimal position computed");
	if (position == nextPosition) {
		LOG_ERROR("GameBotsDaemon::playBotTurn : AI failed to move player " << playerId << " (at position " << position << ") to a valid position at game " << gameId);
		return;
	}
	
	// Actual move
	jsonservice::DefaultPackage package("game.move");
	boost::optional<Game> game  = GameController::gameMove(gameId, nextPosition, userId, package.getHeader());
	if (!game) {
		LOG_ERROR("GameBotsDaemon::playBotTurn : unable to move player " << playerId << " (at position " << position << ") to a valid position at game " << gameId);
		return;
	}
	
	// Notify the group
	notifications::NotificationData payLoad;
	payLoad.addLiteralField("package_type", "p", "notification.game");
	payLoad.addNumericField("game_id", "g", game->getId());
	payLoad.addNumericField("time_stamp", "ts", game->getTimeStamp());
	notifications::NotificationController::singleton().pushGroupNotification(
		notifications::NotificationGroupTypeGame, game->getId(), payLoad);
}


}
}
