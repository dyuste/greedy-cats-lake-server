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
#ifndef YUCODE_MODEL_GAME_H
#define YUCODE_MODEL_GAME_H

#include "services/json/transportdata/model/ModelInterface.h"
#include "Cell.h"
#include "Player.h"
#include <string>

namespace yucode {
namespace restfulgame {
	
class Game : public jsonservice::ModelInterface {
public:
	Game()
	: jsonservice::ModelInterface(0), id_(0), sequenceNum_(0),
	  width_(0), height_(0), ownerUserId_(0), turnPlayerId_(0),
	  randomGame_(false), finished_(false), timeStamp_(0), cells_(), players_() {
		  
	}
	
	Game(unsigned long long id, unsigned long long sequenceNum, 
	     unsigned short width, unsigned short height, unsigned long long ownerUserId, 
	     unsigned long long turnPlayerId, bool randomGame, bool finished, long long timeStamp)
	: jsonservice::ModelInterface(id), id_(id), sequenceNum_(sequenceNum),
	  width_(width), height_(height), ownerUserId_(ownerUserId), turnPlayerId_(turnPlayerId),
	  randomGame_(randomGame), finished_(finished), timeStamp_(timeStamp), cells_(), players_() {
		
	}
	
	inline unsigned long long getId() const { return id_; }
	inline unsigned long long getSequenceNum() const { return sequenceNum_; }
	inline unsigned long long getOwnerUserId() const { return ownerUserId_; }
	inline unsigned short getWidth() const { return width_; }
	inline unsigned short getHeight() const { return height_; }
	inline unsigned long long getTurnPlayerId() const { return turnPlayerId_; }
	inline const std::vector<Cell> & getCells() const { return cells_; }
	inline const std::vector<Player> & getPlayers() const { return players_; }
	inline long long getTimeStamp() const { return timeStamp_; }
	
	inline void setId(unsigned long long id) { id_ = id; setKey(id_); }
	inline void setSequenceNum(unsigned long long sequenceNum) { sequenceNum_ = sequenceNum; }
	inline void setOwnerUserId(unsigned long long ownerUserId) { ownerUserId_ = ownerUserId; }
	inline void setWidth(unsigned short width) { width_ = width; }
	inline void setHeight(unsigned short height) { height_ = height; }
	inline void setTurnPlayerId(unsigned long long turnPlayerId) { turnPlayerId_ = turnPlayerId; }
	inline void setCells(const std::vector<Cell> & cells) { cells_ = cells; }
	inline void setPlayers(const std::vector<Player> & players) { players_ = players; }
	inline void setTimeStamp(long long timeStamp) { timeStamp_ = timeStamp; }

protected:
	void initDictionary();
	
private:
	unsigned long long id_;
	unsigned long long sequenceNum_;
	unsigned long long ownerUserId_;
	unsigned short width_;
	unsigned short height_;
	unsigned long long turnPlayerId_;
	bool randomGame_;
	long long timeStamp_;
	bool finished_;
	std::vector<Cell> cells_;
	std::vector<Player> players_;
public:
	std::ostream& writeJson(std::ostream&, bool minified) const;
};

}
}

#endif
