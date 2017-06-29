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
#include "RestFulController.h"

#include <string>
#include <sstream>

#include "misc/Utilities.h"
#include "database/TableTraverser.h"
#include "SecurityController.h"

using namespace std;

namespace yucode {
namespace restful {

#include "Names.def"

const string RestFulController::TableRestFulUser = "restful_user";
const string RestFulController::TableRestFulFacebookUser = "restful_facebook_user";
const string RestFulController::TableRestFulSession = "restful_session";
const string RestFulController::Md5SaltToken = "5as1df68wes8e";

void RestFulController::bootStrap() {
	DataBase::singleton().createTableIfNotExists(TableRestFulUser,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"user_name VARCHAR(32) NOT NULL, "
		"pass_md5 VARCHAR(32) NOT NULL, "
		"email VARCHAR(128), "
		"name VARCHAR(64), "
		"dummy TINYINT(1) NOT NULL DEFAULT 1, "
		"creation_date DATETIME, "
		"update_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
		"UNIQUE KEY user_name (user_name),"
		"KEY pass_md5 (pass_md5),"
		"UNIQUE KEY email (email),"
		"KEY name (name)"
	);
	
	DataBase::singleton().createTableIfNotExists(TableRestFulFacebookUser,
		"user_id BIGINT UNSIGNED NOT NULL, "
		"token TEXT NOT NULL, "
		"update_date TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, "
		"KEY user_id (user_id) "
	);
		
	DataBase::singleton().createTableIfNotExists(TableRestFulSession,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY, "
		"user_id  BIGINT UNSIGNED NOT NULL, "
		"ip VARCHAR(45) NOT NULL, "
		"session_key_md5 VARCHAR(32) NOT NULL, "
		"creation_date TIMESTAMP NOT NULL, "
		"KEY user_id (user_id),"
		"KEY session_key_md5 (session_key_md5)"
	);
}

bool RestFulController::availableUserName(const string & userName) {
	// Ensure user does not exist
	TableTraverser usersTraverser(
		MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE user_name='" << userName << "' "));
	TableTraverser::iterator userIt = usersTraverser.begin();
	return userIt == usersTraverser.end();
}

bool RestFulController::availableEmail(const string & email) {
	// Ensure user does not exist
	TableTraverser usersTraverser(
		MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE email='" << email << "' "));
	TableTraverser::iterator userIt = usersTraverser.begin();
	return userIt == usersTraverser.end();
}

std::string RestFulController::makeDummyUserName() {
	std::string userName;
	
	bool found = false;
	
	for (int i = 0; i < 30 && !found; ++i) {
		userName = MKSTRING(Names[rand()%NamesSize] << rand()%1000);
		found = availableUserName(userName);
	}
	
	for (int i = 0; i < 50 && !found; ++i) {
		userName = MKSTRING(Names[rand()%NamesSize] << Names[rand()%NamesSize] << rand()%1000);
		found = availableUserName(userName);
	}
	
	for (int i = 0; i < 50 && !found; ++i) {
		userName = MKSTRING(Names[rand()%NamesSize] << Names[rand()%NamesSize] << rand()%100000);
		found = availableUserName(userName);
	}
	
	while (!found) {
		userName = MKSTRING(Names[rand()%NamesSize] << Names[rand()%NamesSize] << rand()%100000000);
		found = availableUserName(userName);
	}
	
	return userName;
}

std::string RestFulController::makeDummyPassMd5() {
	std::string pass = MKSTRING(Names[rand()%NamesSize] << rand()%10000000);
	
	return md5Crypt(pass, Md5SaltToken);
}

boost::optional<string> RestFulController::sessionSignUp(const string & userName, const string & passMd5, const string & email, const string & name, const server::Request& req) {
	// Ensure user does not exist
	std::string userExistsSql;
	if (email.empty()) {
		userExistsSql = MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE user_name='" << userName << "' ");
	} else {
		userExistsSql = MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE user_name='" << userName << "' OR email='" << email << "' ");
	}
	TableTraverser usersTraverser(userExistsSql);
	TableTraverser::iterator userIt = usersTraverser.begin();
	if (userIt != usersTraverser.end())
		return boost::none;
	
	// Insert new user
	string passSaltCrypt = md5Crypt(passMd5+userName, Md5SaltToken);
	time_t timeStamp = time(NULL);
	string vEmail = email.empty()? string("NULL") : ("'" + email + "'");
	string vName = name.empty()? string("NULL") : ("'" + name + "'");
	DataBase::singleton().executeInsert(TableRestFulUser, 
		"user_name,pass_md5,email,name,dummy,creation_date",
		MKSTRING("('" << userName << "','" << passSaltCrypt << "'," << vEmail <<"," << 
		vName << "," << (email.empty()? 1 : 0) << ",FROM_UNIXTIME(" << timeStamp << "))"));
	
	return sessionSignIn(userName, passMd5, req);
}

boost::optional<string> RestFulController::sessionSignIn(const string & userName, const string & passMd5, const server::Request& req) {
	// Retrieve authenticated user id
	string passSaltCrypt = md5Crypt(passMd5+userName, Md5SaltToken);
	
	TableTraverser usersTraverser(
		MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE user_name='" << userName << "' "
			"AND pass_md5='" << passSaltCrypt << "'"));
	TableTraverser::iterator userIt = usersTraverser.begin();
	if (userIt == usersTraverser.end())
		return boost::none;
	
	// Authenticated. Create session token
	unsigned long long userId = userIt.rowAsUnsignedLongLong(0);
	time_t timeStamp = time(NULL);
	string sessionKey = md5Crypt(MKSTRING(userId << timeStamp << req.remote_endpoint), Md5SaltToken) ;
	DataBase::singleton().executeInsert(TableRestFulSession, 
		"user_id,ip,session_key_md5,creation_date",
		MKSTRING("(" << userId << ",'" 
			<< req.remote_endpoint << "','" << sessionKey << "',FROM_UNIXTIME(" << timeStamp << "))"));
	
	return sessionKey;
}

boost::optional<unsigned long long> RestFulController::getAuthenticatedUser(const string & sessionKey, const server::Request& req) {
	// Retrieve session data
	TableTraverser sessionsTraverser(
		MKSTRING("SELECT user_id,ip,UNIX_TIMESTAMP(creation_date) FROM " << TableRestFulSession << " "
			"WHERE session_key_md5='" << sessionKey << "' "));
	TableTraverser::iterator sessionIt = sessionsTraverser.begin();
	if (sessionIt == sessionsTraverser.end())
		return boost::none;
	
	// Validate session
	unsigned long long userId = sessionIt.rowAsUnsignedLongLong(0);
	std::string ip = sessionIt.rowAsString(1);
	time_t timeStamp = sessionIt.rowAsInt(2);
	string requiredSessionKey = md5Crypt(MKSTRING(userId << timeStamp << ip), Md5SaltToken);
	if (requiredSessionKey != sessionKey || ip != req.remote_endpoint)
		return boost::none;
	
	return userId;
}

bool RestFulController::isDummyUser(unsigned long long userId) {
	// Retrieve session data
	TableTraverser sessionsTraverser(
		MKSTRING("SELECT dummy FROM " << TableRestFulUser << " "
			"WHERE id=" << userId << " "));
	TableTraverser::iterator sessionIt = sessionsTraverser.begin();
	if (sessionIt == sessionsTraverser.end()) {
		LOG_ERROR("ResfFulController::isDummyUser : unexpected user not found " << userId);
		return true;
	}
	return sessionIt.rowAsInt(0);
}

bool RestFulController::sessionExtendAccount(unsigned long long userId, const std::string & userName, const std::string & passMd5, const std::string & email, const std::string & name, const server::Request& req) {
	// Ensure user does not exist
	TableTraverser usersTraverser(
		MKSTRING("SELECT id FROM " << TableRestFulUser << " "
			"WHERE (user_name='" << userName << "' OR email='" << email << "') AND id != " << userId));
	TableTraverser::iterator userIt = usersTraverser.begin();
	if (userIt != usersTraverser.end())
		return false;
	
	// Insert new user
	string passSaltCrypt = md5Crypt(passMd5+userName, Md5SaltToken);
	time_t timeStamp = time(NULL);
	string vName = name.empty()? string("NULL") : ("'" + name + "'");
	unsigned long long rows = DataBase::singleton().executeUpdate(TableRestFulUser, 
		MKSTRING("user_name='"<<userName<<"',pass_md5='"<< passSaltCrypt <<"',email='"<< email <<"',name="<< vName <<",dummy=0"),
		MKSTRING("id='" << userId << "'"));
	
	return rows > 0;
}

void RestFulController::sessionExtendFacebook(unsigned long long userId, const std::string & fbToken) {
	DataBase::singleton().executeInsertOrUpdate(
		TableRestFulFacebookUser, 
		"user_id,token", 
		MKSTRING(userId << ",'" << fbToken << "'"), 
		MKSTRING("token='" << fbToken << "'"));
	DataBase::singleton().executeUpdate(TableRestFulUser, 
		MKSTRING("dummy=0"),
		MKSTRING("id='" << userId << "'"));
	

}

}
}
