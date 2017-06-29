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
#include "Log.h"
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>

namespace yucode {
namespace server {
	
__thread long long LogSeqNumber = 0;
__thread unsigned long ThreadId = 0;
const char * LogFile = NULL;
boost::mutex global_stream_lock;

void SetRequestSeqNumber(long long seqNumber) { LogSeqNumber = seqNumber; }

void SetThreadId(long long threadId) { ThreadId = threadId; }

void SetSelfThreadId() {
	std::string threadId = boost::lexical_cast<std::string>(boost::this_thread::get_id());
	unsigned long threadNumber = 0;
	sscanf(threadId.c_str(), "%lx", &threadNumber);
	SetThreadId(threadNumber);
}

void SetLogFile(const char * logFile) { if (!logFile || strlen(logFile) == 0) LogFile = NULL; else LogFile = logFile; }

TimeStamp TimespecDiff(TimeStamp start, TimeStamp end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

TimeStamp TimespecAdd(TimeStamp oldTime, TimeStamp time) {
    if (time.tv_nsec + oldTime.tv_nsec >= 1000000000)
        return (TimeStamp){
            tv_sec: time.tv_sec + oldTime.tv_sec + 1,
            tv_nsec: time.tv_nsec + oldTime.tv_nsec - 1000000000
        };
    else
        return (TimeStamp){
            tv_sec: time.tv_sec + oldTime.tv_sec,
            tv_nsec: time.tv_nsec + oldTime.tv_nsec
        };
}

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

}
}

