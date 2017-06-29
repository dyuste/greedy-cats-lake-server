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
#ifndef _LOG_H_
#define _LOG_H_

#include <iostream>
#include <iomanip>
#include <fstream>
#include <time.h>
#include <boost/thread/mutex.hpp>


namespace yucode {
namespace server {

// Private thread request identifier	
extern __thread long long LogSeqNumber;
extern __thread unsigned long ThreadId;
extern const char * LogFile;
extern boost::mutex global_stream_lock;

void SetLogFile(const char * logFile);
void SetRequestSeqNumber(long long seqNumber);
void SetThreadId(long long threadId);
void SetSelfThreadId();

// Timing
typedef timespec TimeStamp;
typedef struct {
	timespec accum;
	timespec timestamp;
} AccumTimeStamp;

#define SetTimeStamp(TIMESTAMP) \
	clock_gettime(CLOCK_REALTIME, &TIMESTAMP);

#define EnableAccumTimeStamp(TIMESTAMP) \
	clock_gettime(CLOCK_REALTIME, &(TIMESTAMP.timestamp));

#define DisableAccumTimeStamp(TIMESTAMP) \
	{\
		yucode::server::TimeStamp __TIMESTAMP;\
		SetTimeStamp(__TIMESTAMP);\
		TIMESTAMP.accum = yucode::server::TimespecAdd(TIMESTAMP.accum, yucode::server::TimespecDiff(TIMESTAMP.timestamp, __TIMESTAMP));\
	}

#define InitAccumTimeStamp(TIMESTAMP) \
	TIMESTAMP.accum.tv_sec=0;\
	TIMESTAMP.accum.tv_nsec=0;

TimeStamp TimespecDiff(TimeStamp start, TimeStamp end);
TimeStamp TimespecAdd(TimeStamp oldTime, TimeStamp time);
const std::string currentDateTime();

// Logging
#define LOG(...) \
	{yucode::server::global_stream_lock.lock();\
	yucode::server::SetSelfThreadId();\
	std::ofstream outfile;\
	if (yucode::server::LogFile)\
		outfile.open(yucode::server::LogFile, std::fstream::app);\
	std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
	out << yucode::server::currentDateTime() << " LOG RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ << " [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
	if (yucode::server::LogFile) outfile.close();\
	yucode::server::global_stream_lock.unlock();}

#ifdef LOG_VERBOSE_ENABLED
#define LOG_VERBOSE(...) LOG(__VA_ARGS__)
#else
#define LOG_VERBOSE(...) {}
#endif

#define LOG_WARN(...) \
	{yucode::server::global_stream_lock.lock();\
	yucode::server::SetSelfThreadId();\
	std::ofstream outfile;\
	if (yucode::server::LogFile)\
		outfile.open(yucode::server::LogFile, std::fstream::app);\
	std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
	out << yucode::server::currentDateTime() << " WARN RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ << " [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
	if (yucode::server::LogFile) outfile.close();\
	yucode::server::global_stream_lock.unlock();}
	
#define LOG_ERROR(...) \
	{yucode::server::global_stream_lock.lock();\
	yucode::server::SetSelfThreadId();\
	std::ofstream outfile;\
	if (yucode::server::LogFile)\
		outfile.open(yucode::server::LogFile, std::fstream::app);\
	std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
	out << yucode::server::currentDateTime() << " ERROR RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ << " [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
	if (yucode::server::LogFile) outfile.close();\
	yucode::server::global_stream_lock.unlock();}

#define LOG_ERROR_SECURITY(...) \
	{yucode::server::global_stream_lock.lock();\
	yucode::server::SetSelfThreadId();\
	std::ofstream outfile;\
	if (yucode::server::LogFile)\
		outfile.open(yucode::server::LogFile, std::fstream::app);\
	std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
	out << yucode::server::currentDateTime() <<  " ERRORSEC RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ << " [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
	if (yucode::server::LogFile) outfile.close();\
	yucode::server::global_stream_lock.unlock();}
	
#define LOG_ERROR_CRITICAL(...) \
	{yucode::server::global_stream_lock.lock();\
	yucode::server::SetSelfThreadId();\
	std::ofstream outfile;\
	if (yucode::server::LogFile)\
		outfile.open(yucode::server::LogFile, std::fstream::app);\
	std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
	out << yucode::server::currentDateTime() <<  " ERRORCRT RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ << " [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
	if (yucode::server::LogFile) outfile.close();\
	yucode::server::global_stream_lock.unlock();}
	
#define LOG_ELLAPSED(T1, T2, ...) \
	{\
		yucode::server::TimeStamp diff = yucode::server::TimespecDiff(T1, T2);\
		yucode::server::global_stream_lock.lock();\
		yucode::server::SetSelfThreadId();\
		std::ofstream outfile;\
		if (yucode::server::LogFile)\
			outfile.open(yucode::server::LogFile, std::fstream::app);\
		std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
		out << yucode::server::currentDateTime() << " TIMESTAMP RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ \
			<< " (ellapsed " << diff.tv_sec << "." << std::setw(6) << std::setfill('0') \
			<< diff.tv_nsec/1000 << ") [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
		if (yucode::server::LogFile) outfile.close();\
		yucode::server::global_stream_lock.unlock();\
	}

#define LOG_ELLAPSED_SINCE(T1, ...) \
	{\
		yucode::server::TimeStamp __TIMESTAMP;\
		SetTimeStamp(__TIMESTAMP);\
		LOG_ELLAPSED(T1, __TIMESTAMP, __VA_ARGS__)\
	}

#define LOG_ELLAPSED_ACCUM(T1, ...) \
	{\
		yucode::server::global_stream_lock.lock();\
		yucode::server::SetSelfThreadId();\
		std::ofstream outfile;\
		if (yucode::server::LogFile)\
			outfile.open(yucode::server::LogFile, std::fstream::app);\
		std::ostream & out = yucode::server::LogFile? outfile : std::cout;\
		out << yucode::server::currentDateTime() << " TIMESTAMP RQ" << yucode::server::LogSeqNumber << " at TH" << yucode::server::ThreadId << ": " << __VA_ARGS__ \
			<< " (ellapsed " << T1.accum.tv_sec << "." << std::setw(6) << std::setfill('0') \
			<< T1.accum.tv_nsec/1000 << ") [at " << __FILE__ << ":" << __LINE__ << "]" << std::endl;\
		if (yucode::server::LogFile) outfile.close();\
		yucode::server::global_stream_lock.unlock();\
	}

}
}


#endif
	