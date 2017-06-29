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
#ifndef YUCODE_PACKAGE_PACKAGE_H
#define YUCODE_PACKAGE_PACKAGE_H

#include "Header.h"
#include "services/json/transportdata/model/ModelInterface.h"
#include "services/json/RequestContext.h"
#include "misc/Utilities.h"
#include <sstream>

namespace yucode {
namespace jsonservice {

class Package {
public:
	Package(const std::string & packageType = std::string())
		: header_(), packageType_(packageType) {
	}
	
	inline Header &getHeader() { return header_; }
	
	virtual void finalizeHeader() {};
	
	const std::string & getPackageType() const { return packageType_; }
	
	virtual bool hasError() const { return false; }
	virtual std::string errorMessage() const { return std::string(); }
	
	void writeJson(std::ostream &os, const RequestContext * requestContext);
	
	inline std::string toString(const RequestContext * requestContext = NULL) {
		std::ostringstream stm ;
		writeJson(stm, requestContext);
		return stm.str() ;
	}
	
protected:
	virtual void writeContentJson(std::ostream& os, bool minified) {
		os << "1";
	};

protected:
	Header header_;
	std::string packageType_;
};

class DefaultPackage : public Package, public misc::JsonObject {
public:
	DefaultPackage(const std::string & packageType)
		: Package(packageType), JsonObject() {
		}
protected:
	void writeContentJson(std::ostream& os, bool minified) {
		JsonObject::writeJson(os, minified);
	};
private:
	std::vector<std::pair<std::pair<std::string, std::string>, std::string> > fields_;
};

class ErrorPackage : public Package {
public:
	ErrorPackage(const std::string & message, const std::string & packageType)
		: Package(packageType), message_(message) {
		}

protected:
	bool hasError() const { return true; }
	std::string errorMessage() const { return message_; }
	
	void writeContentJson(std::ostream& os, bool minified) {
		os << "{}";
	};
private:
	std::string message_;
};

}
}

#endif
