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
#ifndef YUCODE_SERVER_JSONSERVICEINTERFACE_H
#define YUCODE_SERVER_JSONSERVICEINTERFACE_H

#include <string>
#include <memory>
#include <boost/function.hpp>
#include "server/ServiceInterface.h"
#include "server/Log.h"
#include "services/json/RequestContext.h"
#include "services/json/transportdata/package/Package.h"

namespace yucode {
namespace jsonservice {

	typedef boost::function<void(const server::Request&, server::Reply&, const server::ServerContext &, RequestContext *)> JsonServiceHandler;
	
	class JsonServiceData {
	public:
		JsonServiceData() {}
		virtual ~JsonServiceData() {} //NOTE: Only to ensure v-table exists
	};
	
	class JsonServiceInterface : public server::ServiceInterface {
	// Service Interface
	public:
		void handleRequest(const server::Request& req, server::Reply& rep, const server::ServerContext & con);
	
	// Preparing context from request
	protected:
		virtual RequestContext * makeRequestContext(const server::Request& req);
		
		bool addRequestContext(const server::Request& req, server::Reply& rep, RequestContext * requestContext);
	
		virtual bool addJsonRequestContext(const server::Request& req, server::Reply& rep, RequestContext * requestContext) {
			return true;
		}
		
	// Service handlers
	protected:
		inline bool handlerExists(RequestContext * requestContext) {
			return requestContext && jsonServiceHandlers_.find(requestContext->method) != jsonServiceHandlers_.end();
		}
		
		template<class T>
		void registerJsonServiceHandler(const std::string & method, const JsonServiceHandler &handler, const T &serviceData) {
			jsonServiceHandlers_[method] = handler;
			jsonServiceData_[method] = std::shared_ptr<T>(new T(serviceData));
		}
		
		std::shared_ptr<JsonServiceData> getServiceData(const std::string & method) {
			return jsonServiceData_[method];
		}
		
	private:
		std::map<std::string, JsonServiceHandler> jsonServiceHandlers_;
		std::map<std::string, std::shared_ptr<JsonServiceData> > jsonServiceData_;

	// Handling answer
	protected:
		void responsePackage(server::Reply& rep, const server::Request& req, RequestContext * requestContext, yucode::jsonservice::Package & package);
		
		inline void responseErrorPackage(const server::Request& req, server::Reply& rep, RequestContext * requestContext, const std::string & packageType, const std::string & message) {
				yucode::jsonservice::ErrorPackage package(message, packageType);
				responsePackage(rep, req, requestContext, package);
				LOG("JsonServiceInterface::responseErrorPackage: " << packageType << ":" << message);
		}
	};
}
}

#endif