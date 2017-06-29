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
#include "GameAlgorithm.h"

#include "GameController.h"
#include "misc/Utilities.h"
#include "database/TableTraverser.h"
#include <climits>
#include <unordered_set>

using namespace std;

namespace yucode {
namespace restfulgame {
	
static inline unsigned int PosI(unsigned int POS, unsigned int W) {
	return ((POS)-((POS)/(W))*W);
}

static inline unsigned int PosJ(unsigned int POS, unsigned int W) {
	return ((POS)/(W));
}

static inline unsigned int Pos(unsigned int I, unsigned int J, unsigned int W) {
	return ((J)*(W)+(I));
}

inline bool CanMoveToCell(int I, int J, int width, int height, vector<GameAlgorithm::CellData> & cells) {
	if (I < 0  || J < 0 || I >= height || J >= width) return false;
	
	unsigned int pos = Pos(I,J,width);
	GameAlgorithm::CellData & cell = cells[pos];
	bool canMove = (pos >= 0 && pos < (width*height) && cell.state > 0 && cell.playerId < 0);
	return canMove;
}

vector<GameAlgorithm::CellData> GameAlgorithm::getCells(unsigned long long gameId, unsigned int width, unsigned int height) {
	// Load cells layout
	vector<GameAlgorithm::CellData> cells;
	cells.reserve(width*height);
	
	// Load cell info
	TableTraverser cellTraverser(
		MKSTRING("SELECT position,player_id,resources,state "
			"FROM " << GameController::TableGameCells << " "
			"WHERE game_id=" << gameId << " ORDER BY position ASC"));
	for (TableTraverser::iterator cellIt = cellTraverser.begin(); cellIt != cellTraverser.end(); ++cellIt) {
		unsigned int position = cellIt.rowAsUnsignedInt(0);
		size_t i = position - position/width;
		size_t j = position/width;
		cells.push_back(GameAlgorithm::CellData(
			position, cellIt.isNull(1)? -1 : cellIt.rowAsUnsignedLongLong(1),
			cellIt.rowAsUnsignedInt(2), cellIt.rowAsUnsignedInt(3)));
	}
	
	return cells;
}

vector<GameAlgorithm::PlayerData> GameAlgorithm::getPlayers(unsigned long long gameId) {
	// Load cells layout
	vector<GameAlgorithm::PlayerData> players;
	
	// Load cell info
	TableTraverser playerTraverser(
		MKSTRING("SELECT p.player_id,c.position,p.can_move,p.score,p.user_id,p.virtual_user_id "
			"FROM " << GameController::TableGamePlayers << " p "
			"INNER JOIN " << GameController::TableGameCells << " c ON c.game_id=p.game_id AND c.player_id=p.player_id "
			"WHERE p.game_id=" << gameId << " ORDER BY p.player_id ASC"));
	for (TableTraverser::iterator playerIt = playerTraverser.begin(); playerIt != playerTraverser.end(); ++playerIt) {
		players.push_back(GameAlgorithm::PlayerData(
			playerIt.rowAsUnsignedInt(0), playerIt.rowAsUnsignedLongLong(1),
			playerIt.rowAsUnsignedInt(2) > 0? true : false, playerIt.rowAsInt(3),
			playerIt.isNull(4)? boost::none : boost::optional<unsigned long long>(playerIt.rowAsUnsignedLongLong(4)),
			playerIt.isNull(5)? boost::none : boost::optional<unsigned long long>(playerIt.rowAsUnsignedLongLong(5))));
	}
	
	return players;
}

template <class T>
class MoveTargetCellVisitor {
public:
	void visitAdyacentReachableCells(T & visitor, int fromI, int fromJ, vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height) {
		int i, j;
		bool continueThisWay = true, continueExplore = true;
		
		// Left
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i - 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}

		// Left
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i + 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}

		// Bottom-Left
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i - (1 - j%2);
			j = j - 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}
		
		// Top-Right
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i + j%2;
			j = j + 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}
		
		// Bottom-Right
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i + j%2;
			j = j - 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}
		
		// Top-Left
		continueThisWay = true;
		i = fromI; j = fromJ;
		while(continueThisWay && continueExplore) {
			i = i - (1 - j%2);
			j = j + 1;
			visitAdyacentReachableCell(visitor, i, j, cells, width, height, continueThisWay, continueExplore);
		}
	}
	
	void visitAdyacentReachableCell(T & visitor, int i, int j, vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height, bool & continueThisWay, bool & continueExplore) {
		int pos = Pos(i, j, width);
		if (i < 0 || j < 0 || i >= width || j >= height
			|| cells[pos].state <= 0 || cells[pos].playerId >= 0) {
			continueThisWay = false;
		} else {
			continueThisWay = continueExplore = visitor.visit(i, j, cells, width, height);
		}
	}
};

template <class T>
class ExploreAdyacentCellVisitor {
public:
	void visitAdyacentReachableCells(T & visitor, int fromI, int fromJ, vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height) {
		int i, j;
		int pos = Pos(fromI, fromJ, width);
		if (visited.find(pos) != visited.end())
			return;
		
		visited.insert(pos);
		visitor.visit(fromI, fromJ, cells, width, height);
		
		// Left
		i = fromI; j = fromJ;
		i = i - 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);

		// Right
		i = fromI; j = fromJ;
		i = i + 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);
		
		// Bottom-Left
		i = fromI; j = fromJ;
		i = i - (1 - j%2);
		j = j - 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);
		
		// Top-Right
		i = fromI; j = fromJ;
		i = i + j%2;
		j = j + 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);
		
		// Bottom-Right
		i = fromI; j = fromJ;
		i = i + j%2;
		j = j - 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);
		
		// Top-Left
		i = fromI; j = fromJ;
		i = i - (1 - j%2);
		j = j + 1;
		visitAdyacentReachableCell(visitor, i, j, cells, width, height);
	}
	
	void visitAdyacentReachableCell(T & visitor, int i, int j, vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height) {
		int pos = Pos(i, j, width);
		if (i >= 0 && j >= 0 && i < width && j < height && cells[pos].state > 0)
			visitAdyacentReachableCells(visitor, i, j, cells, width, height);
	}
	
	inline bool isVisited(int pos) { return visited.find(pos) != visited.end(); }
	
private:
	unordered_set<int> visited;
};

bool GameAlgorithm::canMove(unsigned long long turnPlayerId, unsigned int targetPosition, vector<GameAlgorithm::PlayerData> & players, vector<CellData> & cells, int width, int height) {
	unsigned int tI = PosI(targetPosition, width);
	unsigned int tJ = PosJ(targetPosition, width);
	if (tI >= width || tJ >= height)
		return false;
	
	unsigned int playerPosition = players[turnPlayerId].position;	
	unsigned int pI = PosI(playerPosition, width);
	unsigned int pJ = PosJ(playerPosition, width);
	LOG("Moving from " << pI << "-" << pJ << " to " << tI << "-" << tJ);
	
	// Check if target cell is in reachable cells
	class TargetVisitor {
	public:
		TargetVisitor(int _tI, int _tJ) : tI(_tI), tJ(_tJ), foundTargetInReachable(false) {}
		inline bool visit(int i, int j, vector<CellData> & cells, unsigned int width, unsigned int height) {
			bool targetInReachable =  i == tI && j == tJ;
			if (targetInReachable) foundTargetInReachable = true;
			return !targetInReachable;
		}
		inline bool isTargetReachable() { return foundTargetInReachable; }
	private:
		int tI, tJ;
		bool foundTargetInReachable;
	};
	MoveTargetCellVisitor<TargetVisitor> cellsVisitor;
	TargetVisitor visitor(tI, tJ);
	cellsVisitor.visitAdyacentReachableCells(visitor, pI, pJ, cells, width, height);	
	return visitor.isTargetReachable();
}

void GameAlgorithm::move(unsigned long long & turnPlayerId, vector<GameAlgorithm::PlayerData> & players, unsigned int targetPosition, vector<CellData> & cells, int width, int height, bool & endOfGame) {
	unsigned int tI = PosI(targetPosition, width);
	unsigned int tJ = PosJ(targetPosition, width);
	
	unsigned int playerPosition = players[turnPlayerId].position;	
	unsigned int pI = PosI(playerPosition, width);
	unsigned int pJ = PosJ(playerPosition, width);
	
	// Update player score with target cell resources
	if (cells[targetPosition].state == 1) {
		players[turnPlayerId].score += cells[targetPosition].resources;
		players[turnPlayerId].modified = true;
	}
	cells[targetPosition].playerId = turnPlayerId;
	cells[targetPosition].modified = true;
	
	// Update former cell state
	cells[playerPosition].state--;
	cells[playerPosition].playerId = -1;
	cells[playerPosition].modified = true;
	
	// Grab isolated components
	vector<GameAlgorithm::CellComponent> isolatedCellComponents = getIsolatedCellComponents(cells, width, height);

	// Grab resources on isolate cells
	vector<unsigned int> isolatedCellsPositions = getIsolatedCells(isolatedCellComponents);

	if (isolatedCellsPositions.size() > 0) {
		int score = 0;
		for (vector<unsigned int>::const_iterator it = isolatedCellsPositions.begin(); it != isolatedCellsPositions.end(); ++it) {
			score += cells[*it].resources;
			cells[*it].state = 0;
			cells[*it].modified = true;
		}
		players[turnPlayerId].score += score;
		players[turnPlayerId].modified = true;
	}
	
	// Detect isolated players
	for (vector<GameAlgorithm::CellComponent>::const_iterator it = isolatedCellComponents.begin(); it != isolatedCellComponents.end(); ++it) {
		if (it->playerIds.size() == 1 && it->cells.size() > 1) {
			int score = 0;
			for (vector<unsigned long long>::const_iterator cellIt = it->cells.begin(); cellIt != it->cells.end(); ++cellIt) {
				score += cells[*cellIt].resources;
				if (cells[*cellIt].playerId < 0) {
					cells[*cellIt].state = 0;
					cells[*cellIt].modified = true;	
				}
			}
			players[it->playerIds[0]].score += score;
			players[it->playerIds[0]].canMove = false;
			players[it->playerIds[0]].modified = true;
		}
	}

	// Update blocked users's canMove
	vector<unsigned long long> blockedUsers = getBlockedPlayers(cells, width, height);
	if (blockedUsers.size() > 0) {
		for (vector<unsigned long long>::const_iterator it = blockedUsers.begin(); it != blockedUsers.end(); ++it) {
			players[*it].canMove = false;
			players[*it].modified = true;
		}
	}
	
	// Update player turn and detect end of game conditions
	vector<GameAlgorithm::PlayerData>::const_iterator turnPlayerIt = players.begin() + turnPlayerId;
	vector<GameAlgorithm::PlayerData>::const_iterator playerIt; 
	for (playerIt = turnPlayerIt + 1; playerIt != players.end() && !playerIt->canMove; ++playerIt) 
		;
	if (playerIt == players.end())
		for (playerIt = players.begin(); playerIt != turnPlayerIt && !playerIt->canMove; ++playerIt) 
			;
	if (playerIt == turnPlayerIt || playerIt == players.end()) {
		endOfGame = true;
	} else {
		endOfGame = false;
		turnPlayerId = playerIt->playerId;
	}
	
	// At end of game, trunPlayerIt gets remaining available resources if any
	if (endOfGame) {
		int score = 0;
		for (vector<GameAlgorithm::CellData>::iterator it = cells.begin(); it != cells.end(); ++it) {
			if (it->state > 0 && it->playerId < 0) {
				score += it->resources;
				it->state = 0;
			}
		}
		players[turnPlayerId].score += score;
		players[turnPlayerId].modified = true;
	}
}

vector<GameAlgorithm::CellComponent> GameAlgorithm::getIsolatedCellComponents(vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height) {
	// Determine cell components
	class ComponentVisitor {
	public:
		ComponentVisitor(GameAlgorithm::CellComponent & _component)
		  : component(_component) {}
		
		inline void visit(int i, int j, vector<CellData> & cells, unsigned int width, unsigned int height) {
			unsigned long long pos = Pos(i, j, width);
			if (cells[pos].playerId >= 0)
				component.playerIds.push_back(cells[pos].playerId);
			component.cells.push_back(pos);
		}
		
	private:
		GameAlgorithm::CellComponent & component;
	};
	
	ExploreAdyacentCellVisitor<ComponentVisitor> cellsVisitor;
	vector<GameAlgorithm::CellComponent> components;
	for (int pos = 0; pos < width*height; ++pos) {
		if (!cellsVisitor.isVisited(pos) && cells[pos].state > 0) {
			GameAlgorithm::CellComponent component(components.size() + 1);
			ComponentVisitor visitor(component);
			cellsVisitor.visitAdyacentReachableCells(visitor, PosI(pos, width), PosJ(pos, height), cells, width, height);
			components.push_back(component);
		}
	}

	return components;
}
	
vector<unsigned int> GameAlgorithm::getIsolatedCells(vector<GameAlgorithm::CellComponent> & components) {
	vector<unsigned int> isolatedCells;
	
	// Identify the isolate playerless components, and collect cells belonging to it
	for (vector<GameAlgorithm::CellComponent>::const_iterator it = components.begin(); it != components.end(); ++it) {
		//LOG("GameAlgorithm::getIsolatedCells : Detected component with " << it->cells.size() << " cells");
		if (it->playerIds.size() == 0) {
			for (vector<unsigned long long>::const_iterator cellIt = it->cells.begin(); cellIt != it->cells.end(); ++cellIt)
				isolatedCells.push_back(*cellIt);
			//LOG("GameAlgorithm::getIsolatedCells : Isolated component with cells: " << misc::Utilities::implode(it->cells, ","));
		}
	}
	
	return isolatedCells;
}

vector<unsigned long long> GameAlgorithm::getBlockedPlayers(vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height) {
	vector<unsigned long long> blockedPlayers;
	
	// Identify users that cannot move
	class CanMoveVisitor {
	public:
		CanMoveVisitor() : canMove(false) {}
		inline bool visit(int i, int j, vector<CellData> & cells, unsigned int width, unsigned int height) {
			canMove = true;
			return false;
		}
		inline bool existsValidPosition() { return canMove; }
	private:
		bool canMove;
	};
	MoveTargetCellVisitor<CanMoveVisitor> cellsVisitor;
	
	for (unsigned int pos = 0; pos < width*height; ++pos) {
		GameAlgorithm::CellData & cell = cells[pos];
		if (cell.state > 0 && cell.playerId >= 0) {
			CanMoveVisitor visitor;
			cellsVisitor.visitAdyacentReachableCells(visitor, PosI(pos, width), PosJ(pos, width), cells, width, height);
			if (!visitor.existsValidPosition())
				blockedPlayers.push_back(cell.playerId);
		}
	}
#ifdef DEBUG_VERBOSE
	if (blockedPlayers.size() > 0) {
		LOG("GameAlgorithm::getBlockedPlayers : blocked players: " << misc::Utilities::implode(blockedPlayers, ","));
	}
#endif
	return blockedPlayers;
}

unsigned int GameAlgorithm::minMaxGame(unsigned long long gameId, unsigned int width, unsigned int height, unsigned int playerId) {
	// Load cells layout
	vector<CellData> cells = getCells(gameId, width, height);
	vector<PlayerData> players = getPlayers(gameId);
	unsigned int selfPosition = players[playerId].position;
	
	LOG("Run MinMax for " << playerId << " at (" << PosI(selfPosition, width) << ", " << PosJ(selfPosition, width) << ")");
	
	class BotMoveVisitor {
	public:
		BotMoveVisitor(unsigned long long _playerId, unsigned long long _targetPlayerId, const vector<PlayerData> & _initialPlayers, int _recursion) 
			: playerId(_playerId), targetPlayerId(_targetPlayerId),  initialPlayers(_initialPlayers), optimalPlayers(), optimalPosition(-1), optimalScore(INT_MIN), recursion(_recursion) {
		}
			
		inline bool visit(int i, int j, vector<CellData> & initialCells, unsigned int width, unsigned int height) {
			vector<PlayerData> players = initialPlayers;
			vector<CellData> cells = initialCells;
			bool endOfGame = false;
			unsigned long long turnPlayerId = playerId;
			
			// Simulate moving to visited position
			//LOG("Simulate " << turnPlayerId << " moving to (" << i << ", " << "j)");
			move(turnPlayerId, players, Pos(i, j, width), cells, width, height, endOfGame);
			
			while (recursion > 0 && !endOfGame && players[targetPlayerId].canMove && turnPlayerId != targetPlayerId) {
				int turnPlayerPos = players[turnPlayerId].position;

				MoveTargetCellVisitor<BotMoveVisitor> cellsVisitor;
				BotMoveVisitor botMoveVisitor(turnPlayerId, targetPlayerId, players, 0);
				cellsVisitor.visitAdyacentReachableCells(botMoveVisitor, PosI(turnPlayerPos, width), PosJ(turnPlayerPos, width), cells, width, height);
				
				if (botMoveVisitor.getOptimalPosition() >= 0)
					move(turnPlayerId, players, botMoveVisitor.getOptimalPosition(), cells, width, height, endOfGame);
				else
					endOfGame = true;
			}

			if (!endOfGame && recursion > 0) {
				
				// After current move, simulate a recursive game chain
				int turnPlayerPos = players[turnPlayerId].position;
				
				MoveTargetCellVisitor<BotMoveVisitor> cellsVisitor;
				BotMoveVisitor botMoveVisitor(turnPlayerId, targetPlayerId, players, recursion - 1);
				cellsVisitor.visitAdyacentReachableCells(botMoveVisitor, PosI(turnPlayerPos, width), PosJ(turnPlayerPos, width), cells, width, height);
				players = botMoveVisitor.optimalPlayers;
			}
			
			int score = scoreFromPlayers(players) - (players[targetPlayerId].canMove? 0 : remainingScore(cells, width, height));
//			string scoreStr = "";
//			for (vector<PlayerData>::const_iterator it = players.begin(); it != players.end(); ++it) {
//				if (it->playerId != playerId)
//					scoreStr += MKSTRING(" - " <<  it->score);
//				else	
//					scoreStr += MKSTRING(" + " <<  it->score);
//			}
//			LOG("  score is " << score << " if moving to (" << i << ", " << j<<") : " << scoreStr);
			if (score > optimalScore || optimalPlayers.size() == 0) {
//				LOG("Update optimal score to " << score << " moving to (" << i << ", " << j<<")");
				optimalPlayers = players;
				optimalPosition = Pos(i, j, width);
				optimalScore = score;
			}
			
			return true;
		}
		
		inline int getOptimalPosition() const { return optimalPosition; }
		
		inline int remainingScore(const vector<CellData> & cells, unsigned int width, unsigned int height) const {
			int score = 0;
			for (int pos = 0; pos < width*height; ++pos)
				if (cells[pos].state > 0 && cells[pos].playerId == -1)
					score += cells[pos].resources;
			return score;
		}
		
		inline int scoreFromPlayers(const vector<PlayerData> & players) const { 
			int score = players[playerId].score;
			for (vector<PlayerData>::const_iterator it = players.begin(); it != players.end(); ++it) {
				if (it->playerId != playerId)
					score -= it->score;
			}
			return score;
		}
		
	private:
		unsigned long long playerId;
		unsigned long long targetPlayerId;
		const vector<PlayerData> & initialPlayers;
		
		vector<PlayerData> optimalPlayers;
		int optimalPosition;
		int optimalScore;
		
		bool greedy;
		int recursion;
	};
	
	MoveTargetCellVisitor<BotMoveVisitor> cellsVisitor;
	BotMoveVisitor botMoveVisitor(playerId, playerId, players, MaxMinMaxIterations);
	cellsVisitor.visitAdyacentReachableCells(botMoveVisitor, PosI(selfPosition, width), PosJ(selfPosition, width), cells, width, height);
	LOG("Discovered optimal position for " << playerId << " at (" << PosI(botMoveVisitor.getOptimalPosition(), width) << ", " << PosJ(botMoveVisitor.getOptimalPosition(), width) << ")");
 
	return botMoveVisitor.getOptimalPosition();
}

}
}
