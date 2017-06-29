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
#ifndef YUCODE_MODEL_PLAYER_H
#define YUCODE_MODEL_PLAYER_H

#include "services/json/transportdata/model/ModelInterface.h"
#include <string>
#include <boost/optional.hpp>

namespace yucode {
namespace restfulgame {

class Player : public jsonservice::ModelInterface {
public:
	Player()
	: jsonservice::ModelInterface(0), playerId_(0), userId_(boost::none), 
	  virtualUserId_(boost::none), canMove_(true), score_(0) {
		
	}
	
	Player(unsigned long long playerId, const boost::optional<unsigned long long>& userId, 
	       const boost::optional<unsigned long long>& virtualUserId, bool canMove, unsigned int score)
	: jsonservice::ModelInterface(0), playerId_(playerId), userId_(userId),
	  virtualUserId_(virtualUserId), canMove_(canMove), score_(score) {
		
	}
	
	inline boost::optional<unsigned long long> getUserId() const { return userId_; }

protected:
	void initDictionary();
	
private:
	unsigned long long playerId_;
	boost::optional<unsigned long long> userId_;
	boost::optional<unsigned long long> virtualUserId_;
	bool canMove_;
	unsigned int score_;
	
public:
	std::ostream& writeJson(std::ostream&, bool minified) const;
};

}
}

#endif
