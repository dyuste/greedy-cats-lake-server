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
#ifndef MISC_UTILITIES_H_
#define MISC_UTILITIES_H_

#include <string>
#include <sstream>
#include <vector>
#include <math.h>

namespace yucode {
namespace misc {
	class Utilities {
	public:
		template < typename T > 
		static inline std::string to_string( const T& n ) {
			std::ostringstream stm ;
			stm << n ;
			return stm.str() ;
		}
		
		static inline void stringReplace(std::string& str, const std::string& from, const std::string& to) {
			if(from.empty())
				return;
			size_t start_pos = 0;
			while((start_pos = str.find(from, start_pos)) != std::string::npos) {
				str.replace(start_pos, from.length(), to);
				start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
			}
		}
		
		static inline std::string& stringSQLScape(std::string& str) {
			stringReplace(str, "'", "\\'");
			return str;
		}
		
		static inline std::string& scapeJson(std::string& str) {
			stringReplace(str, "\\", "\\\\");
			//stringReplace(str, "'", "\\'"); See http://stackoverflow.com/questions/2275359/jquery-single-quote-in-json-response
			stringReplace(str, "\"", "\\\"");
			return str;
		}
		
		template<typename T>
		static inline std::string implode(const std::vector<T>& collection, const std::string& separator = std::string(), const std::string& decorator = std::string()) {
			std::stringstream ss;
			std::string sep;
			
			for (typename std::vector<T>::const_iterator it = collection.begin(); it != collection.end(); ++it) {
				ss << sep << decorator << *it << decorator;
				sep = separator;
			}
			return ss.str();
		}
		
		static inline std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
			std::stringstream ss(s);
			std::string item;
			while (std::getline(ss, item, delim)) {
				if (!item.empty() && (item.size() > 1 || item[0] != delim))
					elems.push_back(item);
			}
			return elems;
			}


		static inline std::vector<std::string> split(const std::string &s, char delim) {
			std::vector<std::string> elems;
			split(s, delim, elems);
			return elems;
		}
		
		static inline float distanceLatLngToKilometers(float lat1, float lng1, float lat2, float lng2) {
			float kmLat1 = 110.54 * lat1;
			float kmLat2 = 110.54 * lat2;
			float kmLng1 = 111.320 * cos(lat1);
			float kmLng2 = 111.320 * cos(lat2);
			return sqrt((kmLat1-kmLat2)*(kmLat1-kmLat2)+(kmLng1-kmLng2)*(kmLng1-kmLng2));
		}
		
		
	};
}
}

#define MKSTRING(...)\
	({std::stringstream _SS;\
	 _SS << __VA_ARGS__;\
	std::string _STR = _SS.str(); _STR;})
	
#endif
