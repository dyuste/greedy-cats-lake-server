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
#ifndef YUCODE_RESTFUL_GAMEBOTSDAEMON_H
#define YUCODE_RESTFUL_GAMEBOTSDAEMON_H

#include <vector>
#include <string>

namespace yucode {
namespace restfulgame {

class GameBotsDaemon {
public:
	
// BootStrap
public:
	void bootStrap();
protected:
	void bootStrapDataBase();
	void bootStrapLoadBots();
	static const std::string TableDaemons;

private:
	void createBot(const std::string & userName, const std::string & name);

// Daemon services
public:
	void runRandomGamesJoiner();
	void runBotTurnPlayer();
	
// Bot joins game
protected:
	void joinRandomGames();
	unsigned long long getAvailableBotId();
	
private:
	std::vector<unsigned long long> bots_;
	
// Bot plays
protected:
	std::vector<unsigned long long> getBotTurnGames();
	void playBotTurn(unsigned long long gameId);
	
};

}
}

#endif
