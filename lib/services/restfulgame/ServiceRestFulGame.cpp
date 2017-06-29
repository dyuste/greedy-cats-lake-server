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
#include "ServiceRestFulGame.h"

#include <string>
#include <sstream>
#include <boost/bind.hpp>
#include <cmath>

#include "database/TableTraverser.h"
#include "server/Log.h"
#include "misc/Utilities.h"
#include "controller/GameController.h"
#include "notifications/NotificationController.h"

using namespace std;
using namespace yucode::server;
using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {

	ServiceRestFulGame::ServiceRestFulGame() {
		registerRestFulServiceHandler("game.get", /*game_id:UInt64 -> game_id:UInt64 */ 
				    boost::bind(&ServiceRestFulGame::serviceGameGet, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("game.get.updated", /*game_ids:[UInt64],time_stamps[UInt64] -> updated_game_ids:[UInt64] */ 
				    boost::bind(&ServiceRestFulGame::serviceGameGetUpdated, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("game.create", /* user_ids:[UInt64],local_user_names:[String] -> game_id:UInt64 */
				    boost::bind(&ServiceRestFulGame::serviceGameCreate, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("game.move", /* game_id:UInt64,position:UInt64 -> game_id:UInt64 */
				    boost::bind(&ServiceRestFulGame::serviceGameMove, this, _1, _2, _3, _4), true);
		registerRestFulServiceHandler("game.join.random", /* -> game_id:UInt64 */
				    boost::bind(&ServiceRestFulGame::serviceGameRandom, this, _1, _2, _3, _4), true);
		
		registerRestFulServiceHandler("user.search", /* pattern:String -> pattern:String,user_ids:[UInt64] */
				    boost::bind(&ServiceRestFulGame::serviceUserSearch, this, _1, _2, _3, _4), true);
		
		registerRestFulServiceHandler("profile.get.summary", /* -> ready_game_ids:[UInt64],waiting_game_ids:[UInt64],finished_game_ids:[UInt64] */
				    boost::bind(&ServiceRestFulGame::serviceProfileGetSummary, this, _1, _2, _3, _4), true);
	}
	
// Service Interfce
	bool ServiceRestFulGame::matchRequest(const Request& req) const {
		return req.uri == "/games/greedycats/api";
	}
	
	void ServiceRestFulGame::bootStrap() {
		GameController::bootStrap();
		
		// super call
		ServiceRestFul::bootStrap();
	}
	
// Service Restful
	void ServiceRestFulGame::userDidSignUp(const server::Request& req, server::Reply& rep, const server::ServerContext & serverContext, jsonservice::RequestContext * requestContext) {
		GameController::createBomUser(requestContext->userId);
	}
	
	void ServiceRestFulGame::userWasUpdated(unsigned long long userId, jsonservice::Package & package) {
		GameController::addUserToHeader(userId, userId, package.getHeader());
	}
	
// Services
	void ServiceRestFulGame::serviceGameGet(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch and verify arguments
		unsigned long long gameId = 0;
		if (!req.loadArgumentVerifyUnsignedLongLong(gameId, "game_id")) {
			responseErrorPackage(req, rep, requestContext, "game.get", "Missing argument: required 'game_id'");
			return;
		}
		
		// Check for rights
		if (!GameController::userHasReadRightsOverGame(gameId, requestContext->userId)) {
			responseErrorPackage(req, rep, requestContext, "game.get", "Operation not allowed");
			return;
		}
		
		// Fetch game data
		DefaultPackage package("game.get");
		boost::optional<Game> game  = GameController::getGame(requestContext->userId, gameId, package.getHeader());
		if (!game) {
			responseErrorPackage(req, rep, requestContext, "game.get", MKSTRING("Game not found for id " << gameId));
			return;
		}
		
		// Finalize and send
		package.getHeader().addModelObject("games", *game);
		package.addNumericField("game_id", "g", gameId);
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFulGame::serviceGameGetUpdated(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		vector<unsigned long long> gameIds, timeStamps;
		
		if (!req.loadArgumentVerifyCsvUnsignedLongLong(gameIds, "game_ids")
			|| !req.loadArgumentVerifyCsvUnsignedLongLong(timeStamps, "time_stamps")) {			
			responseErrorPackage(req, rep, requestContext, "game.get.updated", "Missing argument: required 'game_ids','time_stamps'");
			return;
		}
		
		if (gameIds.size() != timeStamps.size() || gameIds.size() == 0) {
			responseErrorPackage(req, rep, requestContext, "game.get.updated", "Wrong arguments format (empty lists or not matching sizes)");
			return;
		}
		
		map<unsigned long long, unsigned long long> gameTimeStamps;
		for (size_t it = 0; it < gameIds.size(); ++it)
			gameTimeStamps.insert(make_pair<unsigned long long, unsigned long long>(
				(unsigned long long)gameIds[it], (unsigned long long)timeStamps[it]));
		
		DefaultPackage package("game.get.updated");
		gameIds = GameController::getUpdatesSinceTimeStamp(requestContext->userId,  gameTimeStamps, package.getHeader());
		
		
		// Finalize and send
		package.addCsvNumericField("game_ids", "g", gameIds);
		responsePackage(rep, req, requestContext, package);
	}
	
	
	void ServiceRestFulGame::serviceGameCreate(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch and verify arguments
		vector<unsigned long long> userIds;
		req.loadArgumentVerifyCsvUnsignedLongLong(userIds, "user_ids");
		
		vector<string> localUserNames;
		req.loadArgumentVerifyCsvStringText(localUserNames, "local_user_names");
		
		// first users check
		if ((userIds.size() < 1 && localUserNames.size() < 1) || (userIds.size() + localUserNames.size()) > 20) {
			responseErrorPackage(req, rep, requestContext, "game.create", "Wrong user lists: Required 'user_ids' and/or 'local_user_names', with up to 20 players");
			return;
		}
		
		// Fetch game data
		DefaultPackage package("game.create");
		boost::optional<Game> game  = GameController::createGame(userIds, requestContext->userId, localUserNames, false, package.getHeader());
		if (!game) {
			responseErrorPackage(req, rep, requestContext, "game.create", MKSTRING("Failed to create game"));
			return;
		}
		
		// Finalize and send		
		package.getHeader().addModelObject("games", *game);
		package.addNumericField("game_id", "g", game->getId());
		responsePackage(rep, req, requestContext, package);
		
		// Create Notification Group
		const vector<Player> & players = game->getPlayers();
		for (vector<Player>::const_iterator it = players.begin(); it != players.end(); ++it) {
			if (it->getUserId())
				notifications::NotificationController::singleton().addUserToGroup(
					notifications::NotificationGroupTypeGame, game->getId(), *it->getUserId());
		}
		
		// Notify the group
		notifications::NotificationData payLoad;
		payLoad.addLiteralField("package_type", "p", "notification.game");
		payLoad.addNumericField("game_id", "g", game->getId());
		payLoad.addNumericField("time_stamp", "ts", game->getTimeStamp());
		notifications::NotificationController::singleton().pushGroupNotification(
			notifications::NotificationGroupTypeGame, game->getId(), payLoad);
	}
	
	void ServiceRestFulGame::serviceGameMove(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch and verify arguments
		unsigned long long gameId = 0;
		unsigned int position = 0;
		if (!req.loadArgumentVerifyUnsignedLongLong(gameId, "game_id")
			|| !req.loadArgumentVerifyUnsignedInt(position, "position")) {
			responseErrorPackage(req, rep, requestContext, "game.move", "Missing argument: required 'game_id', 'position'");
			return;
		}
		
		// Do move or fail
		DefaultPackage package("game.move");
		boost::optional<Game> game  = GameController::gameMove(gameId, position, requestContext->userId, package.getHeader());
		if (!game) {
			responseErrorPackage(req, rep, requestContext, "game.move", MKSTRING("Move failed to " << gameId));
			return;
		}
		
		// Finalize and send
		package.getHeader().addModelObject("games", *game);
		package.addNumericField("game_id", "g", gameId);
		responsePackage(rep, req, requestContext, package);
		
		// Notify the group
		notifications::NotificationData payLoad;
		payLoad.addLiteralField("package_type", "p", "notification.game");
		payLoad.addNumericField("game_id", "g", game->getId());
		payLoad.addNumericField("time_stamp", "ts", game->getTimeStamp());
		notifications::NotificationController::singleton().pushGroupNotification(
			notifications::NotificationGroupTypeGame, game->getId(), payLoad);
	}
	
	void ServiceRestFulGame::serviceGameRandom(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch game data
		DefaultPackage package("game.join.random");
		boost::optional<Game> game  = GameController::createOrJoinRandomGame(requestContext->userId, package.getHeader());
		if (!game) {
			responseErrorPackage(req, rep, requestContext, "game.join.random", MKSTRING("Failed to create or join game"));
			return;
		}
		
		// Finalize and send		
		package.getHeader().addModelObject("games", *game);
		package.addNumericField("game_id", "g", game->getId());
		responsePackage(rep, req, requestContext, package);
		
		// Create Notification Group
		const vector<Player> & players = game->getPlayers();
		for (vector<Player>::const_iterator it = players.begin(); it != players.end(); ++it) {
			if (it->getUserId())
				notifications::NotificationController::singleton().addUserToGroup(
					notifications::NotificationGroupTypeGame, game->getId(), *it->getUserId());
		}
		
		// Notify the group
		notifications::NotificationData payLoad;
		payLoad.addLiteralField("package_type", "p", "notification.game");
		payLoad.addNumericField("game_id", "g", game->getId());
		payLoad.addNumericField("time_stamp", "ts", game->getTimeStamp());
		notifications::NotificationController::singleton().pushGroupNotification(
			notifications::NotificationGroupTypeGame, game->getId(), payLoad);
	}
	
	void ServiceRestFulGame::serviceUserSearch(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch and verify arguments
		string pattern;
		if (!req.loadArgumentVerifyStringText(pattern, "pattern")) {
			responseErrorPackage(req, rep, requestContext, "user.search", "Missing argument: required 'pattern'");
			return;
		}
		
		// Fetch game data
		DefaultPackage package("user.search");
		vector<unsigned long long> userIds = GameController::searchUser(requestContext->userId, pattern, package.getHeader());
		
		// Finalize and send
		package.addLiteralField("pattern", "p", pattern);
		package.addCsvNumericField("user_ids", "u", userIds);
		
		responsePackage(rep, req, requestContext, package);
	}
	
	void ServiceRestFulGame::serviceProfileGetSummary(const Request& req, Reply& rep, const ServerContext & serverContext, RequestContext * requestContext) {
		// Fetch game data
		DefaultPackage package("profile.get.summary");
		vector<unsigned long long> waitingGames = GameController::getWaitingGames(requestContext->userId, package.getHeader());
		vector<unsigned long long> readyGames = GameController::getReadyGames(requestContext->userId, package.getHeader());
		vector<unsigned long long> finishedGames = GameController::getFinishedGames(requestContext->userId, package.getHeader());
		
		// Finalize and send
		GameController::addUserToHeader(requestContext->userId, requestContext->userId, package.getHeader());
		package.addCsvNumericField("ready_game_ids", "r", readyGames);
		package.addCsvNumericField("waiting_game_ids", "w", waitingGames);
		package.addCsvNumericField("finished_game_ids", "f", finishedGames);
		responsePackage(rep, req, requestContext, package);
	}
}
}
