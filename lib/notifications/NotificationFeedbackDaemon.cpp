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
#include "NotificationFeedbackDaemon.h"

#include "NotificationResources.h"

#include <boost/thread.hpp>
#include <boost/date_time.hpp>

#include "external/ios/apn/apn.h"
#include "database/TableTraverser.h"
#include "misc/Utilities.h"
#include "server/Log.h"

#define NOTIFCATIONS_ENABLED

using namespace std;
using namespace yucode;

namespace yucode {
namespace notifications {
	
void NotificationFeedbackDaemon::run() {
	boost::posix_time::seconds sleepTime(4*60*60);
	LOG("NotificationFeedbackDaemon:run:  Started NotificationFeedbackDaemon daemon");
	
	while (1) {
		LOG("NotificationFeedbackDaemon:run:  Trigger purgeDeadTokens");
		purgeDeadTokens();
		boost::this_thread::sleep(sleepTime);
	}
}

void NotificationFeedbackDaemon::purgeDeadTokens() {
	vector<string> deadTokens = fetchIosDeadTokens();
	if (deadTokens.size() > 0) {
		LOG("NotificationFeedbackDaemon::purgeDeadTokens : deleting " << deadTokens.size() << " tokens");
		DataBase::singleton().executeQuery( 
			MKSTRING("DELETE FROM " << NotificationController::TableUserTokens << " "
				"WHERE token_type=" << NotificationTokenType::NotificationIosToken << 
				" AND token IN (" << misc::Utilities::implode(deadTokens, ",", "'") << ")"));
	}
}

vector<string> NotificationFeedbackDaemon::fetchIosDeadTokens(){
	vector<string> stdTokens;
	
#ifdef NOTIFCATIONS_ENABLED	
	apn_ctx_ref ctx = NULL;
	apn_error_ref error;
	char **tokens = NULL;
	uint32_t tokens_count = 0;
	uint32_t i = 0;
	
	const char *cert_path = NotificationResources::getIosCertificatePath().c_str();
	const char *key_path = NotificationResources::getIosKeyPath().c_str();
	const char *key_passwd = NotificationResources::getIosKeyPassword().c_str();

	if(apn_init(&ctx, cert_path, key_path, key_passwd, &error) == APN_ERROR){
		LOG_ERROR("NotificationFeedbackDaemon::fetchIosDeadTokens: failed to init APN " <<  error->message << " - " << error->code);
		apn_error_free(&error);
		return stdTokens;
	}

	apn_set_mode(ctx, APN_MODE_SANDBOX, NULL);

	if(apn_feedback_connect(ctx, &error) == APN_ERROR) {
		LOG_ERROR("NotificationFeedbackDaemon::fetchIosDeadTokens: failed Apple Feedback Service" <<  error->message << " - " << error->code);
		apn_free(&ctx);
		apn_error_free(&error);
		return stdTokens;
	}

	if(apn_feedback(ctx, &tokens, &tokens_count, &error) == APN_ERROR) {
		LOG_ERROR("NotificationFeedbackDaemon::fetchIosDeadTokens: failed to fetch tokens" <<  error->message << " - " << error->code);
		apn_close(ctx);
		apn_free(&ctx);
		apn_error_free(&error);
		return stdTokens;
	} 

	for(i = 0; i < tokens_count; i++)
		stdTokens.push_back(string(tokens[i]));

	apn_feedback_tokens_array_free(tokens, tokens_count);    
	apn_close(ctx);
	apn_free(&ctx);
#endif
	
	return stdTokens;
}

}
}
