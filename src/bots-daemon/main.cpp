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
#include <iostream>
#include <string>
#include <cstring>
#include <thread>

#include "database/DataBase.h"
#include "services/restfulgame/controller/GameBotsDaemon.h"
#include "server/Log.h"

using namespace std;
using namespace yucode;

struct Config {
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
	const char *FIELD;
#include "options.def"
#undef OPTION
} config;

bool loadConfig(Config & config, int argc, char* argv[]) {
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
	config.FIELD = DEFAULT;
#include "options.def"
#undef OPTION
	if (!(argc % 2))
		return false;
	
	int i = 1;
	while (i + 1 < argc) {
		char * arg = argv[i];
		char * value = argv[i+1];
		
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
		if (strcmp(arg, ARG_SHORT) == 0 || strcmp(arg, ARG_LARGE) == 0) config.FIELD = value; else
#include "options.def"
		// Final 'else' fallback
		return false;
		i += 2;
#undef OPTION
	}
	return true;
}

void joinerDaemon() {
	// Run daemon
	restfulgame::GameBotsDaemon daemon;
	daemon.bootStrap();
	daemon.runRandomGamesJoiner();	
}

int main (int argc, char ** argv) {
	// Check command line arguments
	if (!loadConfig(config, argc, argv))
	{
		cerr << "Usage: " << argv[0] << " <options>\n";
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
		cerr << "\t" ARG_LARGE "|" ARG_SHORT "\t - " DESC "(def " DEFAULT ")" << endl;
#include "options.def"
#undef OPTION
		return 1;
	}
		
	if (config.log_file && strlen(config.log_file) > 0)
		server::SetLogFile(config.log_file);
	
	try {
		DataBase::singleton().setAccess(config.db_database, config.db_user, config.db_password);
		DataBase::singleton().openConnection();

		thread joinerThread(joinerDaemon);		
	
		// Run daemon
		restfulgame::GameBotsDaemon daemon;
		daemon.bootStrap();
		daemon.runBotTurnPlayer();
		
	} catch (exception& e) {
		cerr << "exception: " << e.what() << "\n";
	}

	return 0;
}
