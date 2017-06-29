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
#ifndef YUCODE_MISC_JSON_OBJECT_H
#define YUCODE_MISC_JSON_OBJECT_H

#include <iostream>
#include <string>
#include <cstdlib>
#include <memory>
#include <boost/optional.hpp>
#include "misc/Utilities.h"

namespace yucode {
namespace misc {

class JsonObject {
	typedef std::shared_ptr<JsonObject> JsonObjectSharedPtr;
	typedef std::vector<JsonObjectSharedPtr> JsonObjectSharedPtrCollection;
	typedef std::pair<std::string, std::string> FieldNamePair;
	typedef std::vector<std::pair<FieldNamePair, std::string> > MappedFieldCollection;
	typedef std::vector<std::pair<FieldNamePair, JsonObjectSharedPtrCollection> > MappedObjectCollection;
	
public:
	JsonObject() : fields_(), objFields_(), dictionaryDidInit_(false) {
	}
	
	void removeField(const std::string &field) {
		MappedFieldCollection::iterator fieldIt;
		for (fieldIt = fields_.begin(); fieldIt != fields_.end(); ++fieldIt) {
			if (fieldIt->first.first == field)
				break;
		}
		if (fieldIt != fields_.end())
			fields_.erase(fieldIt);
		else {
			MappedObjectCollection::iterator objIt;
			for (objIt = objFields_.begin(); objIt != objFields_.end(); ++objIt) {
				if (objIt->first.first == field)
					break;
			}
			if (objIt != objFields_.end())
				objFields_.erase(objIt);
		}
	}
	template <typename T>
	void addNumericField(const std::string &field, const std::string &minField, const T& value) {
		fields_.push_back(std::make_pair<FieldNamePair, std::string>(
			std::make_pair<std::string, std::string>((std::string)field, (std::string)minField), 
							(std::string)MKSTRING(value)));
	}
	
	template <typename T>
	void addLiteralField(const std::string &field, const std::string &minField, const T& value) {
		std::string strVal = escapeJson(MKSTRING(value));
		strVal = strVal.empty()? std::string("null") : MKSTRING("\"" << strVal << "\"");
		fields_.push_back(std::make_pair<FieldNamePair, std::string>(
			std::make_pair<std::string, std::string>((std::string)field,(std::string) minField), 
							(std::string)strVal));
	}
	
	template <typename T>
	void addNumericField(const std::string &field, const std::string &minField, const boost::optional<T> & value) {
		if (value) {
			fields_.push_back(std::make_pair<FieldNamePair, std::string>(
				std::make_pair<std::string, std::string>((std::string)field, (std::string)minField), 
								(std::string)MKSTRING(*value)));
		} else {
			fields_.push_back(std::make_pair<FieldNamePair, std::string>(
				std::make_pair<std::string, std::string>((std::string)field, (std::string)minField), 
								std::string("null")));
		}
	}
	
	template <typename T>
	void addLiteralField(const std::string &field, const std::string &minField,  const boost::optional<T> &  value) {
		if (value) {
			std::string strVal = escapeJson(MKSTRING(*value));
			strVal = strVal.empty()? std::string("null") : MKSTRING("\"" << strVal << "\"");
			fields_.push_back(std::make_pair<FieldNamePair, std::string>(
				std::make_pair<std::string, std::string>((std::string)field,(std::string) minField), 
								(std::string)strVal));
		} else {
			fields_.push_back(std::make_pair<FieldNamePair, std::string>(
				std::make_pair<std::string, std::string>((std::string)field, (std::string)minField), 
								std::string("null")));
		}
	}
	
	template <typename T>
	void addCsvNumericField(const std::string &field, const std::string &minField, const std::vector<T>& value) {
		fields_.push_back(std::make_pair<FieldNamePair, std::string>(
			std::make_pair<std::string, std::string>((std::string)field, (std::string)minField),
				(std::string)MKSTRING("[" << misc::Utilities::implode(value, ",") << "]")));
	}
	
	// FIXME: Each literal should be escaped
	template <typename T>
	void addCsvLiteralField(const std::string &field, const std::string &minField, const std::vector<T>& value) {
		fields_.push_back(std::make_pair<FieldNamePair, std::string>(
			std::make_pair<std::string, std::string>((std::string)field, (std::string)minField),
			(std::string)MKSTRING("[" << misc::Utilities::implode(value, ",", "\"") << "]")));
	}
	
	template <class T>
	void addCsvObjectField(const std::string &field, const std::string &minField, const std::vector<T>& value) {
		typedef typename std::vector<T>::const_iterator T_const_iterator;
		
		JsonObjectSharedPtrCollection objects;
		for (T_const_iterator it = value.begin(); it != value.end(); ++it) {
			JsonObjectSharedPtr object = JsonObjectSharedPtr(new T(*it));
			objects.push_back(object);
		}
		
		objFields_.push_back(
			std::make_pair<FieldNamePair, JsonObjectSharedPtrCollection>(
				std::make_pair<std::string, std::string>((std::string)field, (std::string)minField),
				(JsonObjectSharedPtrCollection) objects
			)
		);
	}
	
	
	virtual void writeJson(std::ostream& os, bool minified) {
		if (!dictionaryDidInit_)
			initDictionary();
		
		os << "{";
		std::string separator = "";
		for (MappedFieldCollection::const_iterator it = fields_.begin(); it != fields_.end(); ++it) {
			if (minified)
				os << separator << "\"" << it->first.second << "\":" << it->second;
			else
				os << separator << "\"" << it->first.first << "\":" << it->second;
			separator = ",";
		}
		for (MappedObjectCollection::const_iterator it = objFields_.begin(); it != objFields_.end(); ++it) {
			if (minified)
				os << separator << "\"" << it->first.second << "\":";
			else
				os << separator << "\"" << it->first.first << "\":";
			writeObjectPointerArrayJson(it->second, os, minified);
			separator = ",";
		}
		os << "}";
	};
	
	std::string toString(bool minified = true) {
		std::stringstream ss;
		writeJson(ss, minified);
		return ss.str();
	}
	
private:
	MappedFieldCollection fields_;
	MappedObjectCollection objFields_;
	bool dictionaryDidInit_;
	
protected:
	virtual void initDictionary() {
	}
	
	void writeObjectPointerArrayJson(const JsonObjectSharedPtrCollection & array, std::ostream& os, bool minified) {
		os << "[";
		std::string separator = "";
		for (JsonObjectSharedPtrCollection::const_iterator it = array.begin(); it != array.end(); ++it) {
			os << separator;
			(*it)->writeJson(os, minified);
			separator = ",";
		}
		os << "]";
	}
	
	static std::string escapeJson(const std::string& input) {
		std::string output;
		output.reserve(input.length());

		for (std::string::size_type i = 0; i < input.length(); ++i)  {
			switch (input[i]) {
			case '"':
				output += "\\\""; break;
			case '/':
				output += "\\/"; break;
			case '\b':
				output += "\\b"; break;
			case '\f':
				output += "\\f";  break;
			case '\n':
				output += "\\n";  break;
			case '\r':
				output += "\\r";   break;
			case '\t':
				output += "\\t";  break;
			case '\\':
				output += "\\\\";  break;
			default:
				output += input[i]; break;
			}
		}
		return output;
	}
};

}
}

#endif
