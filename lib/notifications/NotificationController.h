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
#ifndef YUCODE_NOTIFICATIONS_NOTIFICATION_CONTROLLER_H
#define YUCODE_NOTIFICATIONS_NOTIFICATION_CONTROLLER_H

#include <vector>
#include <string>
#include "misc/JsonObject.h"

namespace yucode {
namespace notifications {

// NOTE: Below code should belong to app domain, 
// but at this point it is fair to place it here
enum NotificationGroupType {
	NotificationGroupTypeGame = 0
};

enum NotificationTokenType {
	NotificationAndroidToken = 0,
	NotificationIosToken
};

class NotificationData : public misc::JsonObject {
public:
	NotificationData()
	 : misc::JsonObject(), optionalPayLoad_(boost::none) {
	}
	
	void addOptionalPayload(const JsonObject & payLoad) {
		optionalPayLoad_ = payLoad;
	}
	
	inline size_t getMaxPayLoadSizeForType(NotificationTokenType type) const {
		switch(type) {
			case NotificationAndroidToken: return 1024; // TODO
			case NotificationIosToken: return 256;
		}
		return 0;
	}
	
	std::string getStringForType(NotificationTokenType type) {
		std::string finalString;
		if (optionalPayLoad_) {
			std::vector<JsonObject> payLoad;
			payLoad.push_back(*optionalPayLoad_);
			addCsvObjectField("pay_load", "p", payLoad);
			
			std::string withPayLoad = toString();
			if (withPayLoad.size() < getMaxPayLoadSizeForType(type))
				finalString = withPayLoad;
			
			removeField("pay_load");
		}
		if (finalString.empty())
			finalString = toString();
		
		return finalString;
	}
	
private:
	boost::optional<JsonObject> optionalPayLoad_;
};

class NotificationController
{
	friend class NotificationFeedbackDaemon;
	
public:
	static const std::string TableUserTokens;
	static const std::string TableGroupUsers;
	
	inline static NotificationController& singleton (){
		static NotificationController instance = NotificationController();
		return instance;
	}
	
protected:
	NotificationController() {
		bootStrap();
	}
	
	void bootStrap();

public:
	void addUserToken(unsigned long long userId, NotificationTokenType tokenType, const std::string & token);
	void addUserToGroup(unsigned int groupTypeId, unsigned long long groupId, unsigned long long userId);
	void removeUserFromGroup(unsigned int groupTypeId, unsigned long long groupId, unsigned long long userId);
	
	void pushUserNotification(unsigned long long userId, NotificationData & payload);
	void pushGroupNotification(unsigned int groupTypeId, unsigned long long groupId, NotificationData & payload);
	
protected:
	void pushIosNotification(const std::vector<std::string> & tokens, NotificationData & payload);
};

} // namespace notificatons
} // namespace yucode

#endif // YUCODE_NOTIFICATIONS_NOTIFICATION_CONTROLLER_H
