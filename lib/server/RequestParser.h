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

#ifndef YUCODE_SERVER_REQUEST_PARSER_H
#define YUCODE_SERVER_REQUEST_PARSER_H

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <string>

#include "Request.h"

namespace yucode {
namespace server {

struct Request;

/// Parser for incoming Requests.
class RequestParser
{
public:
	/// Construct ready to parse the Request method.
	RequestParser();

	/// Reset to initial parser state.
	void reset();

	/// Parse some data. The tribool return value is true when a complete Request
	/// has been parsed, false if the data is invalid, indeterminate when more
	/// data is required. The InputIterator return value indicates how much of the
	/// input has been consumed.
	template <typename InputIterator>
	boost::tuple<boost::tribool, InputIterator> parse(Request& req,
			InputIterator begin, InputIterator end)
	{    
		SetRequestSeqNumber(req.seq_number);
		
		while (begin != end)
		{
			boost::tribool result = consume(req, *begin++);
			if (result || !result) {
				if (result)
					result = enhace_Request(req);
				return boost::make_tuple(result, begin);
			}
		}
		boost::tribool result = boost::indeterminate;
		return boost::make_tuple(result, begin);
	}
	
	static std::string ParseUrlDomain(const std::string & url_s);
	
	// fake parser for debug only
	void parseGetString(Request& req, const std::string & get_request);

private:
	/// Decode uri and extract meaningfull fields.
	bool enhace_Request(Request & req);
	
	/// Decode and map cookies
	bool parse_cookies(Request & req, const std::string& cookies);
	
	/// Decode uri and extract meaningfull fields.
	bool url_decode(const std::string& in, std::string& out);
	
	/// Handle the next character of input.
	boost::tribool consume(Request& req, char input);

	/// Check if a byte is an HTTP character.
	static bool is_char(int c);

	/// Check if a byte is an HTTP control character.
	static bool is_ctl(int c);

	/// Check if a byte is defined as an HTTP tspecial character.
	static bool is_tspecial(int c);

	/// Check if a byte is a digit.
	static bool is_digit(int c);

	/// The current state of the parser.
	enum state
	{
		method_start,
		method,
		uri,
		http_version_h,
		http_version_t_1,
		http_version_t_2,
		http_version_p,
		http_version_slash,
		http_version_major_start,
		http_version_major,
		http_version_minor_start,
		http_version_minor,
		expecting_newline_1,
		header_line_start,
		header_lws,
		header_name,
		space_before_header_value,
		header_value,
		expecting_newline_2,
		expecting_newline_3
	} state_;
};

} // namespace server
} // namespace yucode

#endif // YUCODE_SERVER_REQUEST_PARSER_H