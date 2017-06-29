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
#include "NotificationController.h"

#include "NotificationResources.h"
#include "external/ios/apn/apn.h"
#include "database/TableTraverser.h"
#include "misc/Utilities.h"

//#define NOTIFCATIONS_ENABLED

using namespace std;

namespace yucode {
namespace notifications {
	
const string NotificationController::TableUserTokens = "notification_user_tokens";
const string NotificationController::TableGroupUsers = "notification_group_users";

void NotificationController::bootStrap() {
	DataBase::singleton().createTableIfNotExists(TableUserTokens,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
		"user_id BIGINT UNSIGNED NOT NULL,"
		"token_type INT UNSIGNED NOT NULL,"
		"token TEXT NOT NULL,"
		"time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
		"KEY user_id(user_id),"
		"KEY token(token(48))");
	
	DataBase::singleton().createTableIfNotExists(TableGroupUsers,
		"id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT PRIMARY KEY,"
		"group_type_id INT UNSIGNED NOT NULL,"
		"group_id BIGINT UNSIGNED NOT NULL,"
		"user_id BIGINT UNSIGNED NOT NULL,"
		"time_stamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
		"KEY type_group(group_type_id,group_id),"
		"KEY user_id(user_id),"
		"UNIQUE(group_type_id,group_id,user_id)");
}

void NotificationController::addUserToken(unsigned long long userId, NotificationTokenType tokenType, const std::string & token) {
	DataBase::singleton().executeInsert(TableUserTokens, 
		"user_id,token_type,token", 
		MKSTRING("(" << userId << "," << tokenType << ",'" << token << "')"));
}

void NotificationController::addUserToGroup(unsigned int groupTypeId, unsigned long long groupId, unsigned long long userId) {
	DataBase::singleton().executeInsertIgnore(TableGroupUsers, 
		"group_type_id,group_id,user_id", 
		MKSTRING("(" << groupTypeId << "," << groupId << "," << userId << ")"));
}

void NotificationController::removeUserFromGroup(unsigned int groupTypeId, unsigned long long groupId, unsigned long long userId) {
	DataBase::singleton().executeQuery( 
		MKSTRING("DELETE FROM " << TableGroupUsers << " "
			"WHERE groupTypeId=" << groupTypeId << " AND groupId=" << groupId << " AND userId=" << userId));
}

void NotificationController::pushUserNotification(unsigned long long userId, NotificationData & payload) {
	std::vector<std::string> tokens;
	TableTraverser iosTokens(
		MKSTRING("SELECT token FROM " << TableUserTokens << " "
			 "WHERE user_id=" << userId << " "
				"AND token_type=" << NotificationIosToken));
	for (TableTraverser::iterator it = iosTokens.begin(); it != iosTokens.end(); ++it)
		tokens.push_back(it.rowAsString(0));
	
	if (tokens.size() > 0)
		pushIosNotification(tokens, payload);
}

void NotificationController::pushGroupNotification(unsigned int groupTypeId, unsigned long long groupId, NotificationData & payload) {
	std::vector<std::string> tokens;
	TableTraverser iosTokens(
		MKSTRING("SELECT u.token FROM " << TableUserTokens << " u "
			 "INNER JOIN " << TableGroupUsers << " g "
				"ON u.id=g.user_id "
			 "WHERE g.group_type_id=" << groupTypeId << " AND g.group_id=" << groupId << " "
				"AND u.token_type=" << NotificationIosToken));
	for (TableTraverser::iterator it = iosTokens.begin(); it != iosTokens.end(); ++it)
		tokens.push_back(it.rowAsString(0));
	
	if (tokens.size() > 0)
		pushIosNotification(tokens, payload);
}

void NotificationController::pushIosNotification(const vector<string> & tokens, NotificationData & payload) {
#ifdef NOTIFCATIONS_ENABLED
	apn_payload_ctx_ref payload_ctx = NULL;
	apn_ctx_ref ctx = NULL;
	apn_error error;
	apn_error * errorRef = &error;
	const char *push_message = payload.getStringForType(NotificationIosToken).c_str();
	
	const char *cert_path = NotificationResources::getIosCertificatePath().c_str();
	const char *key_path = NotificationResources::getIosKeyPath().c_str();
	const char *private_key_pass = NotificationResources::getIosKeyPassword().c_str();
	
	if(apn_init(&ctx, cert_path, key_path, private_key_pass, &errorRef) == APN_ERROR){
		LOG_ERROR("NotificationController::pushIosNotification: " <<  error.message << " - " << error.code)
		return;
	}
	for (vector<string>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
		apn_add_token(ctx, it->c_str(), NULL);

	if(apn_payload_init(&payload_ctx, &errorRef) == APN_ERROR) {
		LOG_ERROR("NotificationController::pushIosNotification: " <<  error.message << " - " << error.code)
		apn_free(&ctx);
		return;
	}
	apn_payload_set_badge(payload_ctx, 10, NULL);
	apn_payload_set_body(payload_ctx, push_message, NULL);
	apn_payload_set_sound(payload_ctx, "default",  NULL);
	apn_payload_add_custom_property_integer(payload_ctx, "int_property", 20, NULL);

	if(apn_connect(ctx, &errorRef) == APN_ERROR) {
		LOG_ERROR("NotificationController::pushIosNotification: Could not connected to Apple Push Notification Servece: " <<  error.message << " - " << error.code)
		apn_payload_free(&payload_ctx);
		apn_free(&ctx);
		return;
	}
	if(apn_send(ctx, payload_ctx, &errorRef) == APN_ERROR) {
		LOG_ERROR("NotificationController::pushIosNotification: Could not sent push: " <<  error.message << " - " << error.code)
			apn_close(ctx);
		apn_payload_free(&payload_ctx);
		apn_free(&ctx);
		return;
	} 
	apn_close(ctx);
	apn_payload_free(&payload_ctx);
	apn_free(&ctx);
#endif
}

}
}

