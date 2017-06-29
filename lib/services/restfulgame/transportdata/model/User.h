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
#ifndef YUCODE_MODEL_USER_H
#define YUCODE_MODEL_USER_H

#include "services/json/transportdata/model/ModelInterface.h"
#include <string>

namespace yucode {
namespace restfulgame {

class User : public jsonservice::ModelInterface {
public:
	User()
	:  jsonservice::ModelInterface(0), id_(0),
	   userName_(), name_(), pictureUrl_(), about_(), lifes_(0), score_(0) {
		   
	}
	User (unsigned long long id,
		const std::string &userName,
		const std::string &email,
		const std::string &name,
		const std::string &pictureUrl,
		unsigned long long theme,
		unsigned int lifes,
		unsigned long score,
		const std::string &about
     	) 
	:  jsonservice::ModelInterface(id), id_(id), email_(email), userName_(userName), name_(name), theme_(theme),
	   pictureUrl_(pictureUrl), lifes_(lifes), score_(score), about_(about), timeStamp_(0) {
		   
	}
	
	User (unsigned long long id,
		const std::string &userName,
		const std::string &email,
		const std::string &name,
		const std::string &pictureUrl,
		unsigned long long theme,
		unsigned int lifes,
		unsigned long score,
		const std::string &about,
		long long timeStamp
     	)  :  jsonservice::ModelInterface(id), id_(id), userName_(userName), email_(email), name_(name),
	   pictureUrl_(pictureUrl), theme_(theme), lifes_(lifes), score_(score), about_(about), timeStamp_(timeStamp) {
	}
	
	inline unsigned long long getId() const { return id_; }
	inline const std::string &getName() const { return name_; }
	inline const std::string &getPictureUrl() const { return pictureUrl_; }
	inline const unsigned int getLifes() const { return lifes_; }
	inline const unsigned int getScore() const { return score_; }
	inline const std::string &getAbout() const { return about_; }
	inline long long getTimeStamp() const { return timeStamp_; }
	
	inline void setId(unsigned long long id) { id_ = id; setKey(id); }
	inline void setName(const std::string & name) { name_ = name; }
	inline void setPictureUrl(const std::string & pictureUrl) { pictureUrl_ = pictureUrl; }
	inline void setLifes(unsigned int lifes) { lifes_ = lifes; }
	inline void setScore(unsigned int score) { score_ = score; }
	inline void setAbout(const std::string & about) { about_ = about; }
	inline void setTimeStamp(long long timeStamp) { timeStamp_ = timeStamp; }
	
protected:
	void initDictionary();
	
private:
	unsigned long long id_;
	std::string userName_;
	std::string email_;
	std::string name_;
	std::string pictureUrl_;
	unsigned long long theme_;
	unsigned int lifes_;
	unsigned long score_;
	std::string about_;
	long long timeStamp_;

public:
	std::ostream& writeJson(std::ostream&, bool minified) const;
};

}
}

#endif
