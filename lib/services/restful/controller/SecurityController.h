
#ifndef YUCODE_SECURITY_MD5_H
#define YUCODE_SECURITY_MD5_H

#include <string>

char * md5Crypt(const char *pw, const char *salt);

static inline std::string md5Crypt(const std::string & pw, const std::string & salt) {
	return std::string(md5Crypt(pw.c_str(), salt.c_str())).substr(0, 32);
}

#endif
