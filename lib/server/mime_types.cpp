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

#include "mime_types.h"

namespace yucode {
namespace server {
namespace mime_types {

struct mapping
{
	const char* extension;
	const char* mime_type;
} mappings[] =
{
	{ "gif", "image/gif" },
	{ "htm", "text/html" },
	{ "html", "text/html" },
	{ "jpg", "image/jpeg" },
	{ "png", "image/png" },
	{ "css", "text/css" },
	{ 0, 0 } // Marks end of list.
};

std::string extension_to_type(const std::string& extension)
{
	for (mapping* m = mappings; m->extension; ++m)
	{
		if (m->extension == extension)
		{
			return m->mime_type;
		}
	}

	return "text/plain";
}

} // namespace mime_types
} // namespace server
} // namespace yucode