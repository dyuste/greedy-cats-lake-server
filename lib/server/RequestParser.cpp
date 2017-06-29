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
#include "RequestParser.h"

#include <sstream>
#include "Request.h"

namespace yucode {
namespace server {

RequestParser::RequestParser()
	: state_(method_start)
{
}

void RequestParser::reset()
{
	state_ = method_start;
}


void RequestParser::parseGetString(Request& req, const std::string & get_request)
{
	req.method = "GET";
	req.uri = get_request;
	req.seq_number = 1;
	enhace_Request(req);
}

std::string RequestParser::ParseUrlDomain(const std::string & url_s) {
	std::string domain_;
	
	const std::string prot_end("://");
	std::string::const_iterator prot_i = search(url_s.begin(), url_s.end(),
						prot_end.begin(), prot_end.end());
	prot_i += 3;
	if (prot_i == url_s.end())
		return std::string();
	std::string::const_iterator path_i = find(prot_i, url_s.end(), '/');
	domain_.assign(prot_i, path_i);
	return domain_;
}


boost::tribool RequestParser::consume(Request& req, char input)
{
	switch (state_)
	{
	case method_start:
		if (!is_char(input) || is_ctl(input) || is_tspecial(input))
		{
			return false;
		}
		else
		{
			state_ = method;
			req.method.push_back(input);
			return boost::indeterminate;
		}
	case method:
		if (input == ' ')
		{
			state_ = uri;
			return boost::indeterminate;
		}
		else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.method.push_back(input);
			return boost::indeterminate;
		}
	case uri:
		if (input == ' ')
		{
			state_ = http_version_h;
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			req.uri.push_back(input);
			return boost::indeterminate;
		}
	case http_version_h:
		if (input == 'H')
		{
			state_ = http_version_t_1;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_t_1:
		if (input == 'T')
		{
			state_ = http_version_t_2;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_t_2:
		if (input == 'T')
		{
			state_ = http_version_p;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_p:
		if (input == 'P')
		{
			state_ = http_version_slash;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_slash:
		if (input == '/')
		{
			req.http_version_major = 0;
			req.http_version_minor = 0;
			state_ = http_version_major_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_major_start:
		if (is_digit(input))
		{
			req.http_version_major = req.http_version_major * 10 + input - '0';
			state_ = http_version_major;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_major:
		if (input == '.')
		{
			state_ = http_version_minor_start;
			return boost::indeterminate;
		}
		else if (is_digit(input))
		{
			req.http_version_major = req.http_version_major * 10 + input - '0';
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_minor_start:
		if (is_digit(input))
		{
			req.http_version_minor = req.http_version_minor * 10 + input - '0';
			state_ = http_version_minor;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case http_version_minor:
		if (input == '\r')
		{
			state_ = expecting_newline_1;
			return boost::indeterminate;
		}
		else if (is_digit(input))
		{
			req.http_version_minor = req.http_version_minor * 10 + input - '0';
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case expecting_newline_1:
		if (input == '\n')
		{
			state_ = header_line_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case header_line_start:
		if (input == '\r')
		{
			state_ = expecting_newline_3;
			return boost::indeterminate;
		}
		else if (!req.headers.empty() && (input == ' ' || input == '\t'))
		{
			state_ = header_lws;
			return boost::indeterminate;
		}
		else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.headers.push_back(Header());
			req.headers.back().name.push_back(input);
			state_ = header_name;
			return boost::indeterminate;
		}
	case header_lws:
		if (input == '\r')
		{
			state_ = expecting_newline_2;
			return boost::indeterminate;
		}
		else if (input == ' ' || input == '\t')
		{
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			state_ = header_value;
			req.headers.back().value.push_back(input);
			return boost::indeterminate;
		}
	case header_name:
		if (input == ':')
		{
			state_ = space_before_header_value;
			return boost::indeterminate;
		}
		else if (!is_char(input) || is_ctl(input) || is_tspecial(input))
		{
			return false;
		}
		else
		{
			req.headers.back().name.push_back(input);
			return boost::indeterminate;
		}
	case space_before_header_value:
		if (input == ' ')
		{
			state_ = header_value;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case header_value:
		if (input == '\r')
		{
			state_ = expecting_newline_2;
			return boost::indeterminate;
		}
		else if (is_ctl(input))
		{
			return false;
		}
		else
		{
			req.headers.back().value.push_back(input);
			return boost::indeterminate;
		}
	case expecting_newline_2:
		if (input == '\n')
		{
			state_ = header_line_start;
			return boost::indeterminate;
		}
		else
		{
			return false;
		}
	case expecting_newline_3:
		return (input == '\n');
	default:
		return false;
	}
}

bool RequestParser::is_char(int c)
{
	return c >= 0 && c <= 127;
}

bool RequestParser::is_ctl(int c)
{
	return (c >= 0 && c <= 31) || (c == 127);
}

bool RequestParser::is_tspecial(int c)
{
	switch (c)
	{
	case '(': case ')': case '<': case '>': case '@':
	case ',': case ';': case ':': case '\\': case '"':
	case '/': case '[': case ']': case '?': case '=':
	case '{': case '}': case ' ': case '\t':
		return true;
	default:
		return false;
	}
}

bool RequestParser::is_digit(int c)
{
	return c >= '0' && c <= '9';
}

bool RequestParser::enhace_Request(Request & req)
{
	// Split get arguments
	std::string uri_path, uri_arguments;
	
	LOG("incomming connection from " << req.remote_endpoint);
	LOG("request uri: " << req.uri);
	
	std::size_t arguments_start = req.uri.find_first_of("?");
	if (arguments_start != std::string::npos) {
		uri_path = req.uri.substr(0, arguments_start);
		uri_arguments = req.uri.substr(arguments_start + 1);
	} else {
		uri_path = req.uri;
	}
	req.uri = uri_path;
	
	// Translate uri.
	if (!url_decode(req.uri, req.decoded_uri))
		return false;
	
	// Determine the file extension.
	std::size_t last_slash_pos = req.decoded_uri.find_last_of("/");
	std::size_t last_dot_pos = req.decoded_uri.find_last_of(".");
	if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
	{
		req.extension = req.decoded_uri.substr(last_dot_pos + 1);
	}

	// Determine the root folder, if any.
	std::size_t first_slash_pos = req.decoded_uri.find("/", 1);
	if (first_slash_pos != std::string::npos)
	{
		req.root_folder = req.decoded_uri.substr(1, first_slash_pos - 1);
	} else {
		req.root_folder = req.decoded_uri.substr(1, req.decoded_uri.length() - 1);
	}
	
	// Extract interesting headers
	for (std::vector<Header>::const_iterator headerIt = req.headers.begin(); headerIt != req.headers.end(); ++headerIt) {
		if (headerIt->name == "Cookie") {
			parse_cookies(req, headerIt->value);
		} else if (headerIt->name == "Referer") {
			req.referer = ParseUrlDomain(headerIt->value);
			LOG("header " << headerIt->name << ": " << headerIt->value << " as domain " << req.referer);
		} else if (headerIt->name == "Origin") {
			req.origin = ParseUrlDomain(headerIt->value);
			LOG("header " << headerIt->name << ": " << headerIt->value<< " as domain " << req.origin);
		}
	}
		
	
	// Extract arguments
	if (!uri_arguments.empty()) {
		std::string arg, deco_arg, value, deco_value;
		
		std::size_t cur_pointer = 0;
		
		while (cur_pointer != std::string::npos && cur_pointer + 1 < uri_arguments.length()) {
			std::size_t next_and = uri_arguments.find("&", cur_pointer+1);
			std::size_t next_equal = uri_arguments.find("=", cur_pointer+1);
			
			// Check syntax
			if (next_equal == std::string::npos
				|| ((int)cur_pointer > (int)next_equal - 1)
				|| (next_and != std::string::npos && (int)next_equal > (int)next_and - 1))
				return false;
			
			// Extract argument-value
			arg = uri_arguments.substr(cur_pointer, next_equal - cur_pointer);
			if (next_and != std::string::npos && (int)next_equal == (int)next_and - 1)
				value = std::string();
			else {
				if (next_and != std::string::npos)
					value = uri_arguments.substr(next_equal + 1, next_and - next_equal - 1);
				else
					value = uri_arguments.substr(next_equal + 1);
			}

			// Decode and insert argument
			if (!url_decode(arg, deco_arg) || !url_decode(value, deco_value))
				return false;
			req.arguments[deco_arg] = deco_value;
			
			cur_pointer = (next_and != std::string::npos? next_and + 1 : next_and);
		}
	}
	
	return true;
}

bool RequestParser::url_decode(const std::string& in, std::string& out)
{
	out.clear();
	out.reserve(in.size());
	for (std::size_t i = 0; i < in.size(); ++i)
	{
		if (in[i] == '%')
		{
			if (i + 3 <= in.size())
			{
				int value = 0;
				std::istringstream is(in.substr(i + 1, 2));
				if (is >> std::hex >> value)
				{
					out += static_cast<char>(value);
					i += 2;
				}
				else
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else if (in[i] == '+')
		{
			out += ' ';
		}
		else
		{
			out += in[i];
		}
	}
	return true;
}

bool RequestParser::parse_cookies(Request & req, const std::string& cookies) {
	size_t pos = 0;
	while (pos != std::string::npos && pos < cookies.size()) {
		size_t it = cookies.find_first_of('=', pos);
		if (it != std::string::npos) {
			std::string name = cookies.substr(pos, it-pos);
			if (it + 1 < cookies.size()) {
				pos = it + 1;
				it = cookies.find_first_of(';', pos);
				if (it != std::string::npos) {
					std::string value = cookies.substr(pos, it-pos);
					req.cookies[name] = value;
					pos = it + 1;
					while (pos < cookies.size() && cookies[pos] == ' ')
						++pos;
				} else {
					if (pos < cookies.size()) {
						std::string value = cookies.substr(pos, cookies.size()-pos);
						req.cookies[name] = value;
						pos = std::string::npos;
					}
					pos = it;
				}
			} else pos = it;
		} else pos = it;
	}
}

} // namespace server
} // namespace yucode
