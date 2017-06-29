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
#ifndef YUCODE_NOTIFICATIONS_NOTIFICATION_RESOURCES_H
#define YUCODE_NOTIFICATIONS_NOTIFICATION_RESOURCES_H

namespace yucode {
namespace notifications {

class NotificationResources {
public:
	static std::string getIosCertificatePath() {
		return std::string("apns_cert.pem");
	}
	
	static std::string getIosKeyPath() {
		return std::string("apns_key.pem");
	}
	
	static std::string getIosKeyPassword() {
		return std::string("test");
	}
	
};

}
}

#endif