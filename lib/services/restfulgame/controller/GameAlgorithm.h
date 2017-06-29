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
#ifndef YUCODE_RESTFUL_GAMEALGORITHM_H
#define YUCODE_RESTFUL_GAMEALGORITHM_H

#include <vector>
#include <string>
#include <boost/optional.hpp>

namespace yucode {
namespace restfulgame {

#define MaxMinMaxIterations 4

class GameAlgorithm {
public:
	struct CellData {
		CellData() : position(0), playerId(0), resources(0), state(0), component(0), explored(0) {}
		CellData(unsigned int pos, int pl, unsigned int res, unsigned int st)
		: position(pos), playerId(pl), resources(res), state(st), component(0), explored(0), modified(false) {}
		
		unsigned int position;
		int playerId;
		unsigned int resources;
		unsigned int state;
		
		unsigned int component;
		bool explored;
		bool modified;
	};
	
	struct CellComponent {
		CellComponent(unsigned int i)
		 : id(i), playerIds(), cells(0) {}
		unsigned int id;
		std::vector<int> playerIds;
		std::vector<unsigned long long> cells;
	};
	
	struct PlayerData {
		PlayerData(int _playerId, unsigned int _position, bool _canMove, int _score, 
			const boost::optional<unsigned long long> & _userId,
			const boost::optional<unsigned long long> & _virtualUserId)
		 : playerId(_playerId), position(_position), canMove(_canMove), score(_score),
		   userId(_userId), virtualUserId(_virtualUserId), modified(false) {
			   
		}
		int playerId;
		unsigned int position;
		bool canMove;
		int score;
		boost::optional<unsigned long long> userId;
		boost::optional<unsigned long long> virtualUserId;
		bool modified;
	};
	
	static std::vector<CellData> getCells(unsigned long long gameId, unsigned int width, unsigned int height);
	static std::vector<PlayerData> getPlayers(unsigned long long gameId);
	
// Game Logic
public:
	static bool canMove(unsigned long long turnPlayerId, unsigned int position, std::vector<PlayerData> & players, std::vector<CellData> & cells, int width, int height);
	static void move(unsigned long long & turnPlayerId, std::vector<PlayerData> & players, unsigned int targetPosition, std::vector<CellData> & cells, int width, int height, bool & endOfGame);
	static void computeExtendedGameLogic(std::vector<CellData> & cells, unsigned int width, unsigned int height, std::vector<unsigned int> & isolatedCells, std::vector<unsigned long long> & blockedUsers);
	
protected:
	static std::vector<GameAlgorithm::CellComponent> getIsolatedCellComponents(std::vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height);
	static std::vector<unsigned int> getIsolatedCells(std::vector<GameAlgorithm::CellComponent> & components);
	static std::vector<unsigned long long> getBlockedPlayers(std::vector<GameAlgorithm::CellData> & cells, unsigned int width, unsigned int height);

// AI contrincants
public:
	static unsigned int minMaxGame(unsigned long long gameId, unsigned int width, unsigned int height, unsigned int playerId);
};

}
}

#endif
