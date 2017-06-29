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
#include "database/DataBase.h"
#include "server/Reply.h"
#include "server/Request.h"
#include "server/RequestParser.h"
#include "server/RequestHandler.h"

#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <map>

using namespace std;
using namespace yucode;

struct Config {
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
	const char *FIELD;
#include "options.def"
#undef OPTION
};


Config config;

/*
 * Console/command line action interface
 */
class ConsoleAction {
public:
	ConsoleAction(const string &cmd, const string &shortcut, const string &help)
		: m_cmd(cmd), m_shortcut(shortcut), m_help(help) {}
	const string &getCmd() { return m_cmd; }
	const string &getShortcut() { return m_shortcut; }
	const string &getHelp() { return m_help; }
	virtual bool doAction(const vector<string> &args) = 0;
private:
	string m_cmd;
	string m_shortcut;
	string m_help;
};

/*
 * Console/command line executor
 */
class Console {
public:
	void addAction(ConsoleAction *action) {
		m_actions[action->getCmd()] = action;
		m_shortcuts[action->getShortcut()] = action;
	}

	void executeAction(const vector<string> args) {
		string cmd = *args.begin();
		map<string, ConsoleAction*>::iterator action = m_actions.find(cmd);
		if (action == m_actions.end())
			action = m_shortcuts.find(cmd);

		if (action != m_actions.end() && action != m_shortcuts.end()) {
			if ((*action).second->doAction(args))
				cout << "success" << endl;
			else
				cout << "failed" << endl;
		} else {
			help();
		}
	}

	void run(const vector<string> & args) {
		cout << "YuCode advisor testing system (batch mode)" << endl;

		if (args.begin() == args.end()) {
			help();
			return;
		}

		executeAction(args);

		cout  << endl;
	}

	void run() {
		cout << "YuCode advisor testing system" << endl
		     << "(type exit, help or another command)" << endl;
		bool command_quit = false;
		while (!command_quit) {
			cout << "> ";

			vector<string> args = read_command();
			if (args.begin() == args.end()) {
				help();
				continue;
			}

			string main_cmd = *args.begin();
			if (main_cmd == "quit" || main_cmd == "exit")
				command_quit = true;
			else
				executeAction(args);

			cout  << endl;
		}
	}

	static vector<string> read_command() {
		vector<string> output;
		char buffer[256];
		cin.getline(buffer, sizeof(buffer));


		string input = string(buffer);
		string::size_type pos = 0;
		while (pos != string::npos) {
			string::size_type new_pos = input.find(" ", pos);
			if (new_pos == string::npos) {
				output.push_back(input.substr(pos));
				pos = new_pos;
			} else {
				output.push_back(input.substr(pos, new_pos - pos));
				pos = new_pos + 1;
			}
		}

		return output;
	}

	void help() {
		cout << "YuCode testing system" << endl;
		cout << "available commands:" << endl;
		for (map<string, ConsoleAction*>::const_iterator it = m_actions.begin(); it != m_actions.end(); ++it) {
			cout << "    " << it->first << " (alias " << it->second->getShortcut() << ")" << endl;
			cout << "\t" << it->second->getHelp() << endl << endl;
		}
		cout << "    exit|quit" << endl;
	}
private:
	map<string, ConsoleAction*> m_actions;
	map<string, ConsoleAction*> m_shortcuts;
};

/*
 * Actions
 */

class MyAction : public ConsoleAction {
public:
	MyAction() : ConsoleAction("my_action", "ma",
		"my_action description.\n"
		"\tExample: my_action") {}

	bool doAction(const vector<string> &args) {
		//TODO my_action
		return true;
	}
};

/*
 * Main program
 */

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

int main (int argc, char ** argv) {
	Console console;
	console.addAction(new MyAction());

	// Check command line arguments
	if (!loadConfig(config, argc, argv))
	{
		std::cerr << "Usage: " << argv[0] << " <options>\n";
#define OPTION(ARG_SHORT, ARG_LARGE, FIELD, DESC, DEFAULT) \
		std::cerr << "\t" ARG_LARGE "|" ARG_SHORT "\t - " DESC "(def " DEFAULT ")" << std::endl;
#include "options.def"
#undef OPTION
		return 1;
	}
		
	if (config.log_file && strlen(config.log_file) > 0)
		yucode::server::SetLogFile(config.log_file);
	
	// Setup databse
	yucode::DataBase::singleton().setAccess(config.db_database, config.db_user, config.db_password);
	yucode::DataBase::singleton().openConnection();
	
	// try batch mode first
	if (argc > 1) {
		vector<string> args;
		bool batchMode = false;
		for (int i = 1; i < argc; ++i) {
			if (batchMode) 
				args.push_back(argv[i]);
			else if (strcmp(argv[i], "--batch") == 0)
				batchMode = true;
		}
		if (batchMode) {
			console.run(args);
			return 0;
		}
	}

	console.run();
	
	return 0;
}
