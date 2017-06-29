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
#include "GameController.h"

#include <string>
#include <sstream>

#include "services/restful/controller/RestFulController.h"
#include "misc/Utilities.h"
#include "database/TableTraverser.h"
#include "transportdata/model/User.h"
#include "transportdata/model/Cell.h"
#include "GameAlgorithm.h"
#include "notifications/NotificationController.h"

using namespace std;
using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {
	
const string GameController::TableGame = "bom_game";
const string GameController::TableUser = "bom_user";
const string GameController::TableVirtualUser = "bom_virtual_user";
const string GameController::TableGamePlayers = "bom_game_players";
const string GameController::TableGameCells = "bom_game_cells";

static inline unsigned int PosI(unsigned int POS, unsigned int W) {
	return ((POS)-((POS)/(W))*W);
}

static inline unsigned int PosJ(unsigned int POS, unsigned int W) {
	return ((POS)/(W));
}

static inline unsigned int Pos(unsigned int I, unsigned int J, unsigned int W) {
	return ((J)*(W)+(I));
}

void GameController::bootStrap() {
	string dataBase = MKSTRING(DataBase::singleton().getDataBaseName());
	DataBase::singleton().createDataBaseIfNotExists(dataBase);
	
	DataBase::singleton().createTableIfNotExists(TableUser,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"picture_url VARCHAR(256) DEFAULT NULL, "
		"theme BIGINT UNSIGNED NOT NULL DEFAULT 0, "
		"lifes INT UNSIGNED NOT NULL DEFAULT 10, "
		"score INT UNSIGNED NOT NULL DEFAULT 0, "
		"about TEXT DEFAULT NULL, "
		"time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP "
	);
	
	DataBase::singleton().createTableIfNotExists(TableVirtualUser,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"related_user_id BIGINT UNSIGNED, "
		"name VARCHAR(32) NOT NULL, "
		"picture_url VARCHAR(256) DEFAULT NULL, "
		"theme BIGINT UNSIGNED NOT NULL DEFAULT 0, "
		"lifes INT UNSIGNED NOT NULL DEFAULT 10, "
		"score INT UNSIGNED NOT NULL DEFAULT 0, "
		"time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
		"KEY related_user_id (related_user_id)"
	);
		
	DataBase::singleton().createTableIfNotExists(TableGame,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"sequence_num BIGINT UNSIGNED DEFAULT 0, "
		"width TINYINT UNSIGNED NOT NULL DEFAULT 0, "
		"height TINYINT UNSIGNED NOT NULL DEFAULT 0, "
		"owner_user_id BIGINT UNSIGNED NOT NULL DEFAULT 0, "
		"turn_player_id BIGINT UNSIGNED NOT NULL DEFAULT 0, "
		"random_game TINYINT DEFAULT 0, "
		"finished TINYINT DEFAULT 0, "
		"time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
		"KEY owner_user_id (owner_user_id),"
		"KEY turn_player_id (turn_player_id)"
	);
	
	DataBase::singleton().createTableIfNotExists(TableGamePlayers,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"player_id BIGINT UNSIGNED NOT NULL, "
		"game_id BIGINT UNSIGNED NOT NULL, "
		"user_id BIGINT UNSIGNED DEFAULT NULL, "
		"virtual_user_id BIGINT UNSIGNED DEFAULT NULL, "
		"can_move TINYINT NOT NULL DEFAULT 1, "
		"score INT UNSIGNED NOT NULL DEFAULT 0, "
		"KEY game_id (game_id),"
		"KEY player_id (player_id),"
		"KEY user_id (user_id),"
		"KEY virtual_user_id (virtual_user_id)"
	);
	
	DataBase::singleton().createTableIfNotExists(TableGameCells,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"game_id BIGINT UNSIGNED NOT NULL, "
		"position SMALLINT UNSIGNED NOT NULL, "
		"player_id BIGINT UNSIGNED DEFAULT NULL, "
		"resources SMALLINT UNSIGNED NOT NULL DEFAULT 5, "
		"state SMALLINT UNSIGNED NOT NULL DEFAULT 1, "
		"KEY game_id (game_id),"
		"KEY player_id (player_id),"
		"KEY position (position),"
		"KEY (game_id,player_id)"
	);
}

//
// Game Package
//

bool GameController::userHasReadRightsOverGame(unsigned long long gameId, unsigned long long clientUserId) {
	if (!gameId || !clientUserId)
		return false;
	
	return DataBase::singleton().existsResultsForQuery(
		MKSTRING("SELECT g.id FROM " << TableGame << " g "
			"INNER JOIN " << TableGamePlayers << " u ON u.game_id = g.id "
			"WHERE g.id=" << gameId << " "
			"AND u.user_id=" << clientUserId));
}

boost::optional<Game> GameController::getGame(unsigned long long clientUserId, unsigned long long gameId, jsonservice::Header & header) {
	boost::optional<Game> game = getGameBase(gameId, header);
	if (!game)
		return boost::none;
	
	addGamePlayers(clientUserId, *game, header);
	
	addGameCells(*game, header);
	
	return game;
}

boost::optional<Game> GameController::getGameBase(unsigned long long gameId, jsonservice::Header & header) {
	// Fetch game data
	string queryGame = MKSTRING("SELECT id,sequence_num,width,height,owner_user_id,turn_player_id,random_game,finished,UNIX_TIMESTAMP(time_stamp) "
		"FROM " << TableGame << " WHERE id=" << gameId);
	TableTraverser gameTraverser(queryGame);
	TableTraverser::iterator gameIt = gameTraverser.begin();
	if (gameIt == gameTraverser.end())
		return boost::none;
	
	return Game(gameIt.rowAsUnsignedLongLong(0), gameIt.rowAsUnsignedLongLong(1), 
			gameIt.rowAsUnsignedInt(2), gameIt.rowAsUnsignedInt(3),
			gameIt.rowAsUnsignedLongLong(4), gameIt.rowAsUnsignedInt(5), 
			gameIt.rowAsUnsignedInt(6), gameIt.rowAsUnsignedInt(7),
			gameIt.rowAsLongLong(8));
}

void GameController::addGamePlayers(unsigned long long clientUserId, Game & game, jsonservice::Header & header) {
	// Fetch users
	vector<Player> players;
	TableTraverser usersTraverser(
		MKSTRING("SELECT u.id,ru.user_name,ru.email,ru.name,u.picture_url,u.theme,u.lifes,u.score,"
				"u.about,GREATEST(UNIX_TIMESTAMP(u.time_stamp),UNIX_TIMESTAMP(ru.update_date)),gu.player_id,gu.can_move,gu.score "
			"FROM " << TableGamePlayers << " gu "
			"LEFT JOIN " << TableUser << " u ON u.id=gu.user_id "
			"LEFT JOIN " << restful::RestFulController::TableRestFulUser << " ru ON ru.id=gu.user_id "
			"WHERE gu.game_id=" << game.getId() << " AND gu.user_id IS NOT NULL "
			"ORDER BY gu.id ASC"));
	
	for (TableTraverser::iterator userIt = usersTraverser.begin(); userIt != usersTraverser.end(); ++userIt) {
		unsigned long long userId = userIt.rowAsUnsignedLongLong(0);
		header.addModelObject("users", 
			User(userId,userIt.rowAsString(1),(clientUserId == userId?userIt.rowAsString(2):string()),
			     userIt.rowAsString(3), userIt.rowAsString(4), userIt.rowAsUnsignedLongLong(5), userIt.rowAsUnsignedInt(6),
			     userIt.rowAsUnsignedInt(7), userIt.rowAsString(8), userIt.rowAsLongLong(9)));
		players.push_back(Player(userIt.rowAsUnsignedLongLong(10), userIt.rowAsUnsignedLongLong(0),
					 boost::none, userIt.rowAsInt(11), userIt.rowAsUnsignedInt(12)));
	}
	
	// Fetch virtual users
	TableTraverser vUsersTraverser(
		MKSTRING("SELECT u.id,u.name,u.picture_url,u.theme,u.lifes,u.score,UNIX_TIMESTAMP(u.time_stamp),"
				"gu.player_id,gu.can_move,gu.score "
			"FROM " << TableGamePlayers << " gu "
			"LEFT JOIN " << TableVirtualUser << " u ON u.id=gu.virtual_user_id "
			"WHERE gu.game_id=" << game.getId() << " AND gu.virtual_user_id IS NOT NULL "
			"ORDER BY gu.id ASC"));
	
	for (TableTraverser::iterator userIt = vUsersTraverser.begin(); userIt != vUsersTraverser.end(); ++userIt) {
		header.addModelObject("virtual_users", 
			User(userIt.rowAsUnsignedLongLong(0), userIt.rowAsString(1), string(),
			     userIt.rowAsString(1), userIt.rowAsString(2), userIt.rowAsUnsignedLongLong(3), userIt.rowAsUnsignedInt(4), 
			     userIt.rowAsUnsignedInt(5), string(), userIt.rowAsLongLong(6)));
		players.push_back(Player(userIt.rowAsUnsignedLongLong(7), boost::none, userIt.rowAsUnsignedLongLong(0), 
					 userIt.rowAsInt(8), userIt.rowAsUnsignedInt(9)));
	}
	
	game.setPlayers(players);
}

void GameController::addGameCells(Game & game, jsonservice::Header & header) {
	// Fetch cells
	vector<Cell> cells;
	string queryCells = MKSTRING("SELECT player_id,resources,state "
		"FROM " << TableGameCells << " "
		"WHERE game_id=" << game.getId() << " "
		"ORDER BY position ASC");
	TableTraverser cellsTraverser(queryCells);
	for (TableTraverser::iterator cellIt = cellsTraverser.begin(); cellIt != cellsTraverser.end(); ++cellIt) {
		cells.push_back(
			Cell(cellIt.isNull(0)? boost::none : boost::optional<unsigned long long>(cellIt.rowAsUnsignedLongLong(0)),
			     cellIt.rowAsUnsignedInt(1),
			     cellIt.rowAsUnsignedInt(2)));
	}
	game.setCells(cells);
}

boost::optional<Game> GameController::createGame(const vector<unsigned long long> &userIds, unsigned long long clientUserId, const std::vector<std::string> & virtualUserNames, bool random, jsonservice::Header & header) {
	vector<unsigned long long> fullUserIds;
	
	// Check for owner user in game players (in order to insert if it is not found)
	vector<unsigned long long>::const_iterator selfUserInList;
	for (selfUserInList = userIds.begin(); selfUserInList != userIds.end(); ++selfUserInList)
		if (*selfUserInList == clientUserId)
			break;
	
	// Owner user is the first
	if (selfUserInList == userIds.end())
		fullUserIds.push_back(clientUserId);
	
	// Local virtual users
	vector<unsigned long long> virtualUsers;
	if (virtualUserNames.size() > 0)
		virtualUsers = getOrCreateVirtualUsersForUser(clientUserId, virtualUserNames);
	
	// Network ones last
	fullUserIds.insert (fullUserIds.end(), userIds.begin(), userIds.end());
	
	// Store game object
	unsigned long long gameId = DataBase::singleton().executeInsert(TableGame, 
		"sequence_num,width,height,owner_user_id,turn_player_id,random_game",
		MKSTRING("(0," << DefaultGameWidth << "," 
			<< DefaultGameHeight << "," << clientUserId << "," << 0 << "," << (random? 1 : 0) << ")"));
	
	// Select random user positions
	map<unsigned int, unsigned long long> userPositions;
	vector<unsigned long long> initialScore;
	for (unsigned long long player = 0; player < virtualUsers.size()+fullUserIds.size(); ++player) {
		unsigned int pos = 0;
		while (userPositions.find(
					pos = Pos(
						2 + (rand() % (DefaultGameWidth - 4)), 
						2 + (rand() % (DefaultGameHeight - 4)), 
						DefaultGameWidth)) 
				!= userPositions.end()
			)
			;

		userPositions[pos] = player;
		initialScore.push_back(rand() % MaxResourcesPerCell + 1);
	}
	
	// Store players
	stringstream ssPlayers;
	string separator = "";
	unsigned long long players = 0;
	for (vector<unsigned long long>::const_iterator it = virtualUsers.begin(); it != virtualUsers.end(); ++it) {
		ssPlayers << separator << "(" << players << "," << gameId << ",NULL," << *it << "," << initialScore[players] << ")";
		players++;
		separator = ",";
	}
	for (vector<unsigned long long>::const_iterator it = fullUserIds.begin(); it != fullUserIds.end(); ++it) {
		ssPlayers << separator << "(" << players << "," << gameId << "," << *it << ",NULL," << initialScore[players] << ")";
		players++;
		separator = ",";
	}
	DataBase::singleton().executeInsert(TableGamePlayers, 
		"player_id,game_id,user_id,virtual_user_id,score",
		ssPlayers.str());
	
	// Store cells
	stringstream ssCells;
	separator = "";
	for (int cellPos = 0; cellPos < DefaultGameWidth * DefaultGameHeight; ++cellPos) {
		unsigned int tI = PosI(cellPos, DefaultGameWidth);
		unsigned int tJ = PosJ(cellPos, DefaultGameWidth);
		bool border1 = (tI <= 0 || tI >= DefaultGameWidth - 1) 
			|| (tJ <= 0 || tJ >= DefaultGameHeight - 1);
		bool border2 = !border1 && ((tI <= 1 || tI >= DefaultGameWidth - 2) 
			|| (tJ <= 1 || tJ >= DefaultGameHeight - 2));
		int state = (!border1 && !border2) ? 1
			: (border1? (rand() % 8 > 5 ? 1 : 0) : (rand() % 8 > 2 ? 1 : 0));
		// Double state cells
		if (state == 1) {
			state += (rand() % 4 > 2 ? 1 : 0);
		}
		ssCells << separator << "(" << gameId << "," << cellPos << ","
			<< (userPositions.find(cellPos) != userPositions.end()? MKSTRING(userPositions[cellPos]) : string("NULL")) << ","
			<< MKSTRING((userPositions.find(cellPos) != userPositions.end()? 
				initialScore[userPositions[cellPos]] :(rand() % MaxResourcesPerCell + 1)))
			<< "," << state << ")";
		separator = ",";
	}
	DataBase::singleton().executeInsert(TableGameCells, 
		"game_id,position,player_id,resources,state",
		ssCells.str());
	
	return getGame(clientUserId, gameId, header);
}

boost::optional<Game> GameController::gameMove(unsigned long long gameId, unsigned int position, unsigned long long clientUserId, jsonservice::Header & header) {
	// Fetch general game data
	TableTraverser gameTraverser(
		MKSTRING("SELECT turn_player_id,owner_user_id,width,height "
			"FROM " << TableGame << " "
			"WHERE id=" << gameId));
	TableTraverser::iterator gameIt = gameTraverser.begin();
	if (gameIt == gameTraverser.end())
		return boost::none;
	
	unsigned long long turnPlayerId = gameIt.rowAsUnsignedLongLong(0);
	unsigned long long ownerUserId = gameIt.rowAsUnsignedLongLong(1);
	unsigned int width = gameIt.rowAsUnsignedInt(2);
	unsigned int height = gameIt.rowAsUnsignedInt(3);
	
	// Fetch players
	vector<GameAlgorithm::PlayerData> players = GameAlgorithm::getPlayers(gameId);
	
	// Detect not in turn
	if (turnPlayerId >= players.size()
		|| (players[turnPlayerId].userId != boost::none && *players[turnPlayerId].userId != clientUserId)
		|| (players[turnPlayerId].userId == boost::none && ownerUserId != clientUserId))
		return boost::none;
	
	// Fetch cells
	vector<GameAlgorithm::CellData> cells = GameAlgorithm::getCells(gameId, width, height);
	
	// Validate move
	if (!GameAlgorithm::canMove(turnPlayerId, position, players, cells, width, height)) {
		LOG("GameController::gameMove : can move failed");
		return boost::none;
	}

	// Actual move
	bool endOfGame = false;
	GameAlgorithm::move(turnPlayerId, players, position, cells, width, height, endOfGame);
	
	// Update game
	DataBase::singleton().executeQuery(MKSTRING(
		"UPDATE " << TableGame << " "
		"SET finished=" << (endOfGame? 1 : 0) << ",sequence_num=sequence_num+1,turn_player_id=" << turnPlayerId << " "
		"WHERE id=" << gameId));
			
	// Update players
	for (vector<GameAlgorithm::PlayerData>::const_iterator it = players.begin(); it != players.end(); ++it)
		if (it->modified)
			DataBase::singleton().executeQuery(MKSTRING(
				"UPDATE " << TableGamePlayers << " "
				"SET score=" << it->score << ",can_move=" << (it->canMove? 1 : 0) << " "
				"WHERE player_id=" << it->playerId << " AND game_id=" << gameId));
	
	// Update cells
	for (vector<GameAlgorithm::CellData>::const_iterator it = cells.begin(); it != cells.end(); ++it)
		if (it->modified)
			DataBase::singleton().executeQuery(MKSTRING(
				"UPDATE " << TableGameCells << " "
				"SET state=" << it->state << ",player_id=" << (it->playerId < 0? string("NULL") : MKSTRING(it->playerId)) << " "
				"WHERE position=" << it->position << " AND game_id=" << gameId));
	
	// On End of Game, update total score of each user (real or virtual)
	if (endOfGame) {
		DataBase::singleton().executeQuery(MKSTRING(
			"UPDATE " << TableUser << " AS u "
			"INNER JOIN " << TableGamePlayers << " AS p "
				"ON p.game_id=" << gameId << " "
					"AND p.user_id=u.id "
			"SET u.score=u.score+p.score"));
		
		DataBase::singleton().executeQuery(MKSTRING(
			"UPDATE " << TableVirtualUser << " AS u "
			"INNER JOIN " << TableGamePlayers << " AS p "
				"ON p.game_id=" << gameId << " "
					"AND p.virtual_user_id=u.id "
			"SET u.score=u.score+p.score"));
	}
	
	return getGame(clientUserId, gameId, header);
}

boost::optional<Game> GameController::createOrJoinRandomGame(unsigned long long clientUserId, jsonservice::Header & header) {
	// Try to insert user into an already existing random game
	DataBase::singleton().executeQuery(MKSTRING(
		"INSERT INTO " << TableGamePlayers << "(player_id, game_id, user_id) "
		"SELECT COUNT(p.player_id),g.id," << clientUserId  << " "
		"FROM " << GameController::TableGame << " g "
		"INNER JOIN " << GameController::TableGamePlayers << " p "
			"ON p.game_id=g.id "
		"WHERE g.random_game=1 "
			"AND NOT EXISTS(SELECT 1 FROM " << GameController::TableGamePlayers << " p2 "
			" WHERE p2.user_id="<< clientUserId <<" AND p2.game_id=g.id) "
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

			string cellConstraint = MKSTRING(
						"(position-floor(position/"<< GameController::DefaultGameWidth <<")*"<< GameController::DefaultGameWidth <<")>2 "
						"AND floor(position/"<< GameController::DefaultGameWidth <<")>2 "
						"AND (position-floor(position/"<< GameController::DefaultGameWidth <<")*"<< GameController::DefaultGameWidth <<")<("<< GameController::DefaultGameWidth <<"-2) "
						"ABD floor(position/"<< GameController::DefaultGameWidth <<")<("<< GameController::DefaultGameWidth <<"-2)");
			DataBase::singleton().executeQuery(MKSTRING(
														"UPDATE " << GameController::TableGameCells << " "
														"SET player_id=" << playerId << " "
														"WHERE game_id= " << gameId << " AND player_id IS NULL AND state>0 "
														"AND (" << cellConstraint << ")"
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
			
			return getGame(clientUserId, gameId, header);
		}
	}
	
	// In case of error (no available game), return a fresh new random one
	return createGame(vector<unsigned long long>(), clientUserId, vector<string>(), true, header);
}

//
// User Package
//
void GameController::createBomUser(unsigned long long userId) {
	unsigned long long theme = rand() % MaxBasicThemes;
	DataBase::singleton().executeInsert(TableUser, 
		"id,theme",
		MKSTRING("(" << userId << ", " << theme << ")"));
}

void GameController::addUserToHeader(unsigned long long clientUserId, unsigned long long userId, jsonservice::Header & header) {
	// Ensure user does not exist
	TableTraverser usersTraverser(
		MKSTRING("SELECT u.id,ru.user_name,ru.email,ru.name,u.picture_url,u.theme,u.lifes,u.score,u.about,GREATEST(UNIX_TIMESTAMP(u.time_stamp),UNIX_TIMESTAMP(ru.update_date)) "
			"FROM " << TableUser << " u "
			"LEFT JOIN " << restful::RestFulController::TableRestFulUser << " ru ON ru.id = u.id "
			"WHERE ru.id=" << userId << " "
			"ORDER BY u.score DESC "));
	usersTraverser.setChunkSize(32);
	
	for (TableTraverser::iterator userIt = usersTraverser.begin(); userIt != usersTraverser.end(); ++userIt) {
		unsigned long long userId = userIt.rowAsUnsignedLongLong(0);
		header.addModelObject("users", 
			User(userId,userIt.rowAsString(1),(clientUserId == userId?userIt.rowAsString(2):string()),userIt.rowAsString(3),
			     userIt.rowAsString(4), userIt.rowAsUnsignedLongLong(5), userIt.rowAsUnsignedInt(6),
			     userIt.rowAsUnsignedInt(7), userIt.rowAsString(8), userIt.rowAsLongLong(9)));
	}
}

vector<unsigned long long> GameController::searchUser(unsigned long long clientUserId, const string & pattern, jsonservice::Header & header) {
	vector<unsigned long long> userIds;
	
	if (pattern.empty()) return userIds;
	
	string whereClause;
	if (pattern.find('@') != string::npos) {
		whereClause = MKSTRING("ru.email LIKE '%" << pattern << "%' ");
	} else {
		string splitPattern = misc::Utilities::implode(misc::Utilities::split(pattern, ' '), "%");
		whereClause = MKSTRING("ru.name LIKE '%" << splitPattern << "%' "
			"OR ru.user_name  LIKE '%" << splitPattern << "%' ");
	}
	
	// Ensure user does not exist
	TableTraverser usersTraverser(
		MKSTRING("SELECT u.id,ru.user_name,ru.email,ru.name,u.picture_url,u.theme,u.lifes,u.score,u.about,GREATEST(UNIX_TIMESTAMP(u.time_stamp),UNIX_TIMESTAMP(ru.update_date)) "
			"FROM " << TableUser << " u "
			"LEFT JOIN " << restful::RestFulController::TableRestFulUser << " ru ON ru.id = u.id "
			"WHERE " << whereClause << " "
			"ORDER BY u.score DESC "));
	usersTraverser.setChunkSize(32);
	
	int count = 0;
	for (TableTraverser::iterator userIt = usersTraverser.begin(); userIt != usersTraverser.end() && count < 30; ++userIt) {
		unsigned long long userId = userIt.rowAsUnsignedLongLong(0);
		header.addModelObject("users",
			User(userId,userIt.rowAsString(1),(clientUserId == userId?userIt.rowAsString(2):string()),userIt.rowAsString(3),
				userIt.rowAsString(4), userIt.rowAsUnsignedLongLong(5), userIt.rowAsUnsignedInt(6),
				userIt.rowAsUnsignedInt(7), userIt.rowAsString(8), userIt.rowAsLongLong(9)));
		userIds.push_back(userIt.rowAsUnsignedLongLong(0));
		++count;
	}
	
	return userIds;
}

vector<unsigned long long> GameController::getOrCreateVirtualUsersForUser(unsigned long long ownerUserId,  const vector<string> & localUserNames) {
	vector<unsigned long long> finalUsers;
	
	vector<string> missingUserNames = localUserNames;
	
	// Fetch already existing virtual users
	TableTraverser usersTraverser(MKSTRING("SELECT v.id, v.name "
		"FROM " << TableVirtualUser << " v "
		"WHERE v.related_user_id = " << ownerUserId << " "
		"AND v.name IN (" << misc::Utilities::implode(localUserNames, ",", "'") << ") "));
	for (TableTraverser::iterator userIt = usersTraverser.begin(); userIt != usersTraverser.end(); ++userIt) {
		finalUsers.push_back(userIt.rowAsUnsignedLongLong(0));
		
		// Remove user names already found as virtual users (of owner user)
		string name = userIt.rowAsString(1);
		vector<string>::iterator userNameIt;
		for (userNameIt = missingUserNames.begin(); userNameIt != missingUserNames.end(); ++userNameIt) 
			if (*userNameIt == name)
				break;
		if (userNameIt != missingUserNames.end())
			missingUserNames.erase(userNameIt);
	}
	
	// Insert missing virtual users
	if (missingUserNames.size() > 0) {
		stringstream ssVirtualUsers;
		string separator = "";
		for (vector<string>::const_iterator it = missingUserNames.begin(); it != missingUserNames.end(); ++it) {
			unsigned long long theme = rand() % MaxBasicThemes;
			ssVirtualUsers << separator << "(" << ownerUserId << ",'" << *it << "'," << theme << ")";
			separator = ",";
		}
		DataBase::singleton().executeInsert(TableVirtualUser, 
			"related_user_id,name,theme",
			ssVirtualUsers.str());
		
		// Fetch just created users
		TableTraverser newUsersTraverser(MKSTRING("SELECT v.id "
			"FROM " << TableVirtualUser << " v "
			"WHERE v.related_user_id = " << ownerUserId << " "
			"AND v.name IN (" << misc::Utilities::implode(missingUserNames, ",", "'") << ") "));
		
		for (TableTraverser::iterator userIt = newUsersTraverser.begin(); userIt != newUsersTraverser.end(); ++userIt)
			finalUsers.push_back(userIt.rowAsUnsignedLongLong(0));
	}
	return finalUsers;
}

vector<unsigned long long> GameController::getReadyGames(unsigned long long clientUserId, jsonservice::Header & header) {
	vector<unsigned long long> gameIds;
	TableTraverser gamesTraverser(MKSTRING(
		"SELECT DISTINCT(g.id) FROM " << TableGame << " g "
		"INNER JOIN " << TableGamePlayers << " p "
			"ON g.turn_player_id=p.player_id AND g.id=p.game_id "
		"WHERE (p.user_id=" << clientUserId << " "
		 "OR (p.user_id IS NULL AND g.owner_user_id=" << clientUserId << ")) "
		"AND g.finished=0 "
		"AND (g.random_game=0 OR (SELECT COUNT(1) FROM " << TableGamePlayers << " pp2 WHERE pp2.game_id=g.id) >= 4)"));
	for (TableTraverser::iterator it = gamesTraverser.begin(); it != gamesTraverser.end(); ++it) {
		unsigned long long gameId = it.rowAsUnsignedLongLong(0);
		boost::optional<Game> game = getGame(clientUserId, gameId, header);
		if (game) {
			gameIds.push_back(gameId);
			header.addModelObject("games", *game);
		}
	}
	return gameIds;
}

vector<unsigned long long> GameController::getWaitingGames(unsigned long long clientUserId, jsonservice::Header & header) {
	vector<unsigned long long> gameIds;
	TableTraverser gamesTraverser(MKSTRING(
		"SELECT DISTINCT(g.id) FROM " << TableGame << " g "
		"INNER JOIN " << TableGamePlayers << " p "
			"ON g.turn_player_id=p.player_id AND g.id=p.game_id "
		"WHERE g.finished=0 "
		"AND (g.owner_user_id=" << clientUserId << " "
			"OR EXISTS(SELECT 1 FROM " << TableGamePlayers << " pp "
					"WHERE pp.game_id=g.id AND pp.user_id=" << clientUserId << ")"
		") AND ("
			"(p.user_id IS NOT NULL AND p.user_id != " << clientUserId << ") "
			"OR (g.random_game=1 AND (SELECT COUNT(1) FROM " << TableGamePlayers << " pp2 WHERE pp2.game_id=g.id) < 4) "
		")"));
	for (TableTraverser::iterator it = gamesTraverser.begin(); it != gamesTraverser.end(); ++it) {
		unsigned long long gameId = it.rowAsUnsignedLongLong(0);
		boost::optional<Game> game = getGame(clientUserId, gameId, header);
		if (game) {
			gameIds.push_back(gameId);
			header.addModelObject("games", *game);
		}
	}
	return gameIds;
}

vector<unsigned long long> GameController::getFinishedGames(unsigned long long clientUserId, jsonservice::Header & header) {
	vector<unsigned long long> gameIds;
	TableTraverser gamesTraverser(MKSTRING(
		"SELECT DISTINCT(g.id) FROM " << TableGame << " g "
		"INNER JOIN " << TableGamePlayers << " p "
			"ON g.id=p.game_id "
		"WHERE p.user_id=" << clientUserId << " "
		"AND g.finished=1 ORDER BY g.time_stamp DESC"));
	int count = 0;
	for (TableTraverser::iterator it = gamesTraverser.begin(); it != gamesTraverser.end() && count < 8; ++it) {
		unsigned long long gameId = it.rowAsUnsignedLongLong(0);
		boost::optional<Game> game = getGame(clientUserId, gameId, header);
		if (game) {
			gameIds.push_back(gameId);
			header.addModelObject("games", *game);
			++count;
		}
	}
	return gameIds;
}

vector<unsigned long long> GameController::getUpdatesSinceTimeStamp(unsigned long long clientUserId, const map<unsigned long long, unsigned long long> & gameTimeStamps, jsonservice::Header & header) {
	vector<unsigned long long> gameIds;
	TableTraverser gamesTraverser(MKSTRING(
		"SELECT g.id, UNIX_TIMESTAMP(g.time_stamp) FROM " << TableGame << " g "
		"WHERE (g.owner_user_id=" << clientUserId << " "
			"OR EXISTS(SELECT 1 FROM " << TableGamePlayers << " pp "
					"WHERE pp.game_id=g.id AND pp.user_id=" << clientUserId << ")"
			") "));
	for (TableTraverser::iterator it = gamesTraverser.begin(); it != gamesTraverser.end(); ++it) {
		unsigned long long gameId = it.rowAsUnsignedLongLong(0);
		unsigned long long timeStamp = it.rowAsUnsignedLongLong(1);
		
		map<unsigned long long, unsigned long long>::const_iterator gameTimeStampsIt;
		gameTimeStampsIt = gameTimeStamps.find(gameId);
		bool required = (gameTimeStampsIt == gameTimeStamps.end()) 
			|| (gameTimeStampsIt->second < timeStamp);
		
		if (required) {
			boost::optional<Game> game = getGame(clientUserId, gameId, header);
			if (game) {
				gameIds.push_back(gameId);
				header.addModelObject("games", *game);
			}
		}
	}
	
	return gameIds;
}

}
}
