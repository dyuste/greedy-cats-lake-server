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
#ifndef CURL_ASIOCURL_H_
#define CURL_ASIOCURL_H_

#include <curl/curl.h>
#include <curl/multi.h>
#include <string>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include "server/Log.h"

using namespace std;

namespace curl {
	
	template <typename CONNECTION_DATA>
	class AsioCurl;

	template <typename CONNECTION_DATA>
	struct AsioCurlConnection {
		AsioCurlConnection(const CONNECTION_DATA& _data) : data(_data)  {}
			
		CURL *easy;
		char *url;
		char error[CURL_ERROR_SIZE];
		string packageString;
		CONNECTION_DATA data;
		yucode::server::TimeStamp start_timestamp;
	};
	
	template <typename CONNECTION_DATA>
	class AsioCurl {
	public:
		AsioCurl(boost::asio::io_service & curlService, boost::asio::io_service & dispatcherService)
			: curlService_(curlService),
			  dispatcherService_(dispatcherService),
			  timer_(curlService),
			  socketMap_() 
		{
			curl_global_init(CURL_GLOBAL_DEFAULT);
			multi_ = curl_multi_init();
		
			curl_multi_setopt(multi_, CURLMOPT_SOCKETFUNCTION, curl::AsioCurl<CONNECTION_DATA>::SocketCallback);
			curl_multi_setopt(multi_, CURLMOPT_SOCKETDATA, this);
			curl_multi_setopt(multi_, CURLMOPT_TIMERFUNCTION, curl::AsioCurl<CONNECTION_DATA>::MultiTimerCallback);
			curl_multi_setopt(multi_, CURLMOPT_TIMERDATA, this);
		}
	
		virtual ~AsioCurl() {
			if (multi_)
				curl_multi_cleanup(multi_);
			curl_global_cleanup();
		}
		
		void newConnection(const std::string &url, CONNECTION_DATA & data){
			AsioCurlConnection<CONNECTION_DATA> *connection;
			CURLMcode rc;
			
			connection = new AsioCurlConnection<CONNECTION_DATA>(data);
			connection->error[0]='\0';
			connection->easy = curl_easy_init();
			connection->url = strdup(url.c_str());
			if ( !connection->easy )
			{
				if (!packageErrorFunction_.empty())
					dispatcherService_.post(boost::bind(packageErrorFunction_, connection->data));
				return;
			}
			curl_easy_setopt(connection->easy, CURLOPT_URL, connection->url);
			curl_easy_setopt(connection->easy, CURLOPT_WRITEFUNCTION, curl::AsioCurl<CONNECTION_DATA>::WriteCallback);
			curl_easy_setopt(connection->easy, CURLOPT_WRITEDATA, connection);
			//curl_easy_setopt(connection->easy, CURLOPT_VERBOSE, 1L);
			curl_easy_setopt(connection->easy, CURLOPT_ERRORBUFFER, connection->error);
			curl_easy_setopt(connection->easy, CURLOPT_PRIVATE, connection);
			curl_easy_setopt(connection->easy, CURLOPT_NOPROGRESS, 1L);
			curl_easy_setopt(connection->easy, CURLOPT_LOW_SPEED_TIME, 60L);
			curl_easy_setopt(connection->easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
			curl_easy_setopt(connection->easy, CURLOPT_SSL_VERIFYHOST, 0);
			curl_easy_setopt(connection->easy, CURLOPT_SSL_VERIFYPEER, 0); 

			/* call this function to get a socket */
			curl_easy_setopt(connection->easy, CURLOPT_OPENSOCKETFUNCTION, curl::AsioCurl<CONNECTION_DATA>::OpenSocketCallback);
			curl_easy_setopt(connection->easy, CURLOPT_OPENSOCKETDATA, this);
			
			/* call this function to close a socket */
			curl_easy_setopt(connection->easy, CURLOPT_CLOSESOCKETFUNCTION, curl::AsioCurl<CONNECTION_DATA>::CloseSocketCallback);
			curl_easy_setopt(connection->easy, CURLOPT_CLOSESOCKETDATA, this);

			LOG("AsioCurl::newConnection : Scheduled query " << url);
			
			SetTimeStamp(connection->start_timestamp);
			rc = curl_multi_add_handle(multi_, connection->easy);
			
			verifyMCodeOrDie("AsioCurl::newConnection: curl_multi_add_handle", rc, connection);
		}
		
		void onCurlError(boost::function<void(std::string where, CURLMcode code, std::string codeStr)> callback) {
			curlErrorFunction_ = callback;
		}
		
		void onPackageRecieved(boost::function<void(std::string package, CONNECTION_DATA connection)> callback) {
			packageRecievedFunction_ = callback;
		}
		
		void onPackageError(boost::function<void(CONNECTION_DATA connection)> callback) {
			packageErrorFunction_ = callback;
		}
	private:
		boost::function<void(std::string where, CURLMcode code, std::string codeStr)> curlErrorFunction_;
		boost::function<void(std::string package, CONNECTION_DATA connection)> packageRecievedFunction_;
		boost::function<void(CONNECTION_DATA connection)> packageErrorFunction_;
		
	protected:
		void checkMultiInfo(){
			char *effectiveURl;
			CURLMsg *msg;
			int messagesLeft;

			while ((msg = curl_multi_info_read(multi_, &messagesLeft)))
			{
				if (msg->msg == CURLMSG_DONE)
				{
					AsioCurlConnection<CONNECTION_DATA> *connection;
					CURL * easy = msg->easy_handle;
					CURLcode res = msg->data.result;
					curl_easy_getinfo(easy, CURLINFO_PRIVATE, &connection);
					curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &effectiveURl);
					
					yucode::server::SetRequestSeqNumber(connection->data->getRequestMessage().seqNumber);
					LOG_ELLAPSED_SINCE(connection->start_timestamp, "AsioCurl: provider service finshed");
					
					// Call to client
					if (!packageRecievedFunction_.empty())
						dispatcherService_.post(boost::bind(packageRecievedFunction_, connection->packageString, connection->data));
					
					curl_multi_remove_handle(multi_, easy);
					
					double tt = 0.0;
					double ns = 0.0;
					double ct = 0.0;
					double pt = 0.0;
					double st = 0.0;
					curl_easy_getinfo(easy, CURLINFO_TOTAL_TIME, &tt);
					curl_easy_getinfo(easy, CURLINFO_NAMELOOKUP_TIME, &ns);
					curl_easy_getinfo(easy, CURLINFO_CONNECT_TIME, &ct);
					curl_easy_getinfo(easy, CURLINFO_PRETRANSFER_TIME, &pt);
					curl_easy_getinfo(easy, CURLINFO_STARTTRANSFER_TIME, &st);
					LOG("AsioCurl: Provider timing info - Total: " << tt << " Lookup: "<< ns << "; Connect: " << ct << "; Pre transfer: " << pt << "; Start transfer: " << st);
					
					free(connection->url);
					curl_easy_cleanup(easy);
					delete connection;
				} else {
					LOG("AsioCurl::checkMultiInfo : Received message: " << msg->msg);
				}
			}
		}
		
		bool verifyMCodeOrDie(const char *where, CURLMcode code, AsioCurlConnection<CONNECTION_DATA> * connection = NULL)
		{
			if ( CURLM_OK != code )
			{
				bool fatal = false;
				const char *s;
				switch ( code )
				{
					case CURLM_CALL_MULTI_PERFORM: s="CURLM_CALL_MULTI_PERFORM"; fatal = true; break;
					case CURLM_BAD_HANDLE:         s="CURLM_BAD_HANDLE"; fatal = true; break;
					case CURLM_BAD_EASY_HANDLE:    s="CURLM_BAD_EASY_HANDLE"; break;
					case CURLM_OUT_OF_MEMORY:      s="CURLM_OUT_OF_MEMORY"; fatal = true; break;
					case CURLM_INTERNAL_ERROR:     s="CURLM_INTERNAL_ERROR"; fatal = true; break;
					case CURLM_UNKNOWN_OPTION:     s="CURLM_UNKNOWN_OPTION"; fatal = true; break;
					case CURLM_LAST:               s="CURLM_LAST"; break;
					case CURLM_BAD_SOCKET:         s="CURLM_BAD_SOCKET"; break;
					default: s="CURLM_unknown"; break;
				}
				
				LOG_ERROR("AsioCurl::verifyMCodeOrDie : error '" << s << "' at " << where);
				
				if (!fatal && connection) {
					if (!packageErrorFunction_.empty())
						dispatcherService_.post(boost::bind(packageErrorFunction_, connection->data));
				} else if (!curlErrorFunction_.empty())
					dispatcherService_.post(boost::bind(curlErrorFunction_, where, code, s));
				
				return false;
			}
			return true;
		}
		
	private:
		
	/// CURL Timer Callback
		static int MultiTimerCallback(CURLM *multi, long timeout_ms, AsioCurl *g)
		{
			// /*VERBOSE*/LOG("CURL: MultiTimerCallback");
			/* cancel running timer */
			g->timer_.cancel();

			//LOG("MultiTimerCallback timeout is " << timeout_ms);
			if ( timeout_ms > 0 )
			{
				// /*VERBOSE*/LOG("CURL: MultiTimerCallback - timeout of " << timeout_ms<<"");
				/* update timer */
				g->timer_.expires_from_now(boost::posix_time::millisec(timeout_ms));
				g->timer_.async_wait(boost::bind(&AsioCurl::TimerCallback, _1, g));
			}
			else if (timeout_ms == 0)
			{
				// /*VERBOSE*/LOG("CURL: MultiTimerCallback - right now timeout");
				/* call timeout function immediately */
				boost::system::error_code error; /*success*/
				AsioCurl::TimerCallback(error, g);
			} else {
				
				// /*VERBOSE*/LOG("CURL: MultiTimerCallback - negative timeout (reset to 100)");
				g->timer_.expires_from_now(boost::posix_time::millisec(1000));
				g->timer_.async_wait(boost::bind(&AsioCurl::TimerCallback, _1, g));
			}
		
			return 0;
		}
		
		static void TimerCallback(const boost::system::error_code & error, AsioCurl *g)
		{
			//DAVID LOG("TimerCallback");
			if (!error)
			{
				// /*VERBOSE*/LOG("TimerCallback (run multi)");
				CURLMcode rc;
				rc = curl_multi_socket_action(g->multi_, CURL_SOCKET_TIMEOUT, 0, &g->stillRunning_);
				
				if (!g->verifyMCodeOrDie("AsioCurl::TimerCallback: curl_multi_socket_action", rc))
					return;
				
				g->checkMultiInfo();

			}
		}
		
	/// Curl Socket Callback
		static int SocketCallback(CURL *curl, curl_socket_t socket, int action, void *callbackDataPointer, void *actionPtr)
		{
			// /*VERBOSE*/LOG("CURL: SocketCallback on socket (" << socket << ")");
			AsioCurl *g = (AsioCurl*) callbackDataPointer;
			
			if (action == CURL_POLL_REMOVE) {
				// /*VERBOSE*/LOG("CURL: SocketCallback - action remove POLL (action " << action << ") (run multi)");
				g->removeSocket((int*)actionPtr, socket, action);
			} else {
				
				if (!actionPtr) {
					// /*VERBOSE*/LOG("CURL: SocketCallback - action POLL (action " << action << ") ADD (run multi)");
					g->addSocket(socket, action, curl);
				} else {
					// /*VERBOSE*/LOG("CURL: SocketCallback - action POLL (action " << action << ") SET (run multi)");
					g->setSocket((int*)actionPtr, socket, action);
				}
			}
			return 0;
		}
		
		void addSocket(curl_socket_t socket, int action, CURL *easy)
		{
			int *actionPtr = (int *)calloc(sizeof(int), 1); /* fdp is used to store current action */
			curl_multi_assign(multi_, socket, actionPtr);
			setSocket(actionPtr, socket, action);
		}
		
		void setSocket(int *actionPtr, curl_socket_t socket, int action)
		{
			*actionPtr = action;
			
			typename std::map<curl_socket_t, boost::asio::ip::tcp::socket*>::iterator it = socketMap_.find(socket);
			if (it != socketMap_.end()) {
				// TODO: for sure I should set true to cancel here
				watchSocket(actionPtr, it->second);
			}
		}
		
		void removeSocket(int *actionPtr, curl_socket_t socket, int action)
		{
			if (actionPtr) {
				typename std::map<curl_socket_t, boost::asio::ip::tcp::socket*>::iterator it = socketMap_.find(socket);
				if (it != socketMap_.end()) {
					*actionPtr = action;
					watchSocket(actionPtr, it->second);
				}
				
				free(actionPtr);
			}
		}
		
	/// ASIO Callbacks (monitoring sockets)
		static void EventCallback(const boost::system::error_code error, AsioCurl * g, boost::asio::ip::tcp::socket * tcpSocket, int notifiedAction, int * actionPtr)
		{
			if (!error)
			{
				// /*VERBOSE*/LOG("ASIO: EventCallback (notified action " << notifiedAction << ", active action " << *actionPtr << "), (run multi)");
				
				CURLMcode rc;
				rc = curl_multi_socket_action(g->multi_, tcpSocket->native_handle(), notifiedAction, &g->stillRunning_);

				if (!g->verifyMCodeOrDie("AsioCurl::EventCallback: curl_multi_socket_action", rc)) {
					return;
				}
				
				g->checkMultiInfo();

				if (g->stillRunning_ <= 0) {
					//LOG("EventCallback : Last transfer done, kill timer");
					// /*VERBOSE*/LOG("ASIO: EventCallback -- cancel timer");
					g->timer_.cancel();
				}
				else {
					// /*VERBOSE*/LOG("ASIO: EventCallback -- watch sockets");
					g->watchSocket(actionPtr, tcpSocket);
				}
			}
		}
		
		void watchSocket(int * actionPtr, boost::asio::ip::tcp::socket *tcpSocket)
		{
			if (tcpSocket->is_open())
			{
				// /*VERBOSE*/LOG("watchSocket (watching " << *actionPtr << "), cancel former and may watch");
				tcpSocket->cancel();
			
				if (*actionPtr == CURL_POLL_IN)
				{
					// /*VERBOSE*/LOG("watchSocket -- POLL IN");
					tcpSocket->async_read_some(boost::asio::null_buffers(),
							boost::bind(&curl::AsioCurl<CONNECTION_DATA>::EventCallback,
								_1,
								this,
								tcpSocket,
								*actionPtr,
								actionPtr));
				}
				else if (*actionPtr == CURL_POLL_OUT)
				{
					// /*VERBOSE*/LOG("watchSocket -- POLL OUT");
					tcpSocket->async_write_some(boost::asio::null_buffers(),
							boost::bind(&curl::AsioCurl<CONNECTION_DATA>::EventCallback,
								_1,
								this,
								tcpSocket,
								*actionPtr,
								actionPtr));
				}
				else if (*actionPtr == CURL_POLL_INOUT)
				{
					// /*VERBOSE*/LOG("watchSocket -- POLL IN/OUT");
					tcpSocket->async_read_some(boost::asio::null_buffers(),
							boost::bind(&curl::AsioCurl<CONNECTION_DATA>::EventCallback,
								_1,
								this,
								tcpSocket,
								*actionPtr,
								actionPtr));
			
					tcpSocket->async_write_some(boost::asio::null_buffers(),
							boost::bind(&curl::AsioCurl<CONNECTION_DATA>::EventCallback,
								_1,
								this,
								tcpSocket,
								*actionPtr,
								actionPtr));
				}
			} else {
				LOG_WARN("AsioCurl::watchSocket : Attempt to watching closed socket");
			}
		}
		
	/// Easy Callbacks
		static size_t WriteCallback(void *incommingData, size_t size, size_t nmemb, void *connectionPtr)
		{
			// /*VERBOSE*/LOG("WriteCallback: " << size*nmemb);
			
			AsioCurlConnection<CONNECTION_DATA> * connection = static_cast<AsioCurlConnection<CONNECTION_DATA> *> (connectionPtr);
			size_t realsize = size * nmemb;
			if (connection && realsize > 0)
				connection->packageString.append(static_cast<const char*>(incommingData), realsize);
			 else {
				LOG_WARN("AsioCurl::WriteCallback : Empty transmission");
			}
			return realsize;
		}
		
		static curl_socket_t OpenSocketCallback(AsioCurl *g, curlsocktype purpose, struct curl_sockaddr *address)
		{
			curl_socket_t sockfd = CURL_SOCKET_BAD;
			// /*VERBOSE*/LOG("OpenSocketCallback");
			
			/* restrict to ipv4 */
			if (purpose == CURLSOCKTYPE_IPCXN && address->family == AF_INET)
			{
				/* create a tcp socket object */
				boost::asio::ip::tcp::socket *tcpSocket = 
					new boost::asio::ip::tcp::socket(g->curlService_);
				
				/* open it and get the native handle*/
				boost::system::error_code ec;
				tcpSocket->open(boost::asio::ip::tcp::v4(), ec);
				boost::asio::ip::tcp::socket::non_blocking_io nb(true);
				tcpSocket->io_control(nb);

				if (ec) {
					LOG_ERROR("AsioCurl::OpenSocketCallback : Error opening socket: " << ec);
					// TODO notify app
					//if (!g->packageErrorFunction_.empty())
					//	g->packageErrorFunction_(connection->data);
				} else {
					sockfd = tcpSocket->native_handle();

					/* save it for monitoring */
					g->socketMap_.insert(std::pair<curl_socket_t, boost::asio::ip::tcp::socket*>(
						sockfd, tcpSocket));
				}	
			}

			// /*VERBOSE*/LOG("OpenSocketCallback opened (" << sockfd << ")");
			return sockfd;
		}
		
		static int CloseSocketCallback(AsioCurl *g, curl_socket_t socket) 
		{
			typename std::map<curl_socket_t, boost::asio::ip::tcp::socket *>::iterator it = g->socketMap_.find(socket);

			// /*VERBOSE*/LOG("CloseSocketCallback (" << socket << ")");
			
			if (it != g->socketMap_.end())
			{
				delete it->second;
				g->socketMap_.erase(it);
			}
			return 0;
		}
		
	protected:
		CURLM *multi_;
		int stillRunning_;
		
	private:
		boost::asio::io_service &curlService_;
		boost::asio::io_service &dispatcherService_;
		boost::asio::deadline_timer timer_;
		std::map<curl_socket_t, boost::asio::ip::tcp::socket *> socketMap_;
	};

}

#endif


