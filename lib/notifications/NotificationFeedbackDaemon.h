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
#ifndef YUCODE_NOTIFICATIONS_NOTIFICATION_FEEDBACK_DAEMON_H
#define YUCODE_NOTIFICATIONS_NOTIFICATION_FEEDBACK_DAEMON_H

#include "NotificationController.h"

#include <vector>
#include <string>
#include "misc/JsonObject.h"

namespace yucode {
namespace notifications {

class NotificationFeedbackDaemon
{
public:
	NotificationFeedbackDaemon() {
	}
	
	void bootStrap() {
		NotificationController::singleton().bootStrap();
	}
	
	void run();
	
protected:
	void purgeDeadTokens();
	
protected:
	std::vector<std::string> fetchIosDeadTokens();
};

} // namespace notificatons
} // namespace yucode

#endif // YUCODE_NOTIFICATIONS_NOTIFICATION_CONTROLLER_H
