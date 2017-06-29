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
#ifndef YUCODE_MODEL_CELL_H
#define YUCODE_MODEL_CELL_H

#include "services/json/transportdata/model/ModelInterface.h"
#include <string>
#include <boost/optional.hpp>

namespace yucode {
namespace restfulgame {

class Cell : public jsonservice::ModelInterface {
public:
	Cell()
	: jsonservice::ModelInterface(0), userId_(0), resources_(0), state_(0) {
		
	}
	
	Cell(const boost::optional<unsigned long long>& userId, unsigned int resources, unsigned int state)
	: jsonservice::ModelInterface(0), userId_(userId), resources_(resources), state_(state) {
		
	}
	
	inline boost::optional<unsigned long long> getUserId() const { return userId_; }
	inline unsigned int getResources() const { return resources_; }
	inline unsigned int getState() const { return state_; }
	
	inline void setUserId(boost::optional<unsigned long long> userId) { userId_ = userId; }
	inline void setResources(unsigned int resources) { resources_ = resources; }
	inline void setState(unsigned int state) { state_ = state; }

protected:
	void initDictionary();
	
private:
	boost::optional<unsigned long long> userId_;
	unsigned int resources_;
	unsigned int state_;
public:
	std::ostream& writeJson(std::ostream&, bool minified) const;
};

}
}

#endif
