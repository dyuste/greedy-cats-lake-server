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
#include "User.h"
#include <iostream>

using namespace yucode::jsonservice;

namespace yucode {
namespace restfulgame {
		
void User::initDictionary() {
	addNumericField("id", "i", id_);
	addLiteralField("user_name", "u", userName_);
	addLiteralField("email", "e", email_);
	addLiteralField("name", "n", name_);
	addNumericField("theme", "t", theme_);
	addLiteralField("picture_url", "p", pictureUrl_);
	addLiteralField("about", "a", about_);
	addNumericField("lifes", "l", lifes_);
	addNumericField("score", "s", score_);
	addNumericField("time_stamp", "ts", timeStamp_);
}

}
}
