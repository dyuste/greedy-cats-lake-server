noinst_LIBRARIES = libyucode-server.a

INCLUDES = @LIBBOOST_CPPFLAGS@ -I$(top_srcdir)/lib

libyucode_server_a_SOURCES = Log.cpp Connection.cpp mime_types.cpp Reply.cpp RequestHandler.cpp RequestParser.cpp Server.cpp ServiceFileDispatch.cpp ServiceInterface.cpp
libyucode_server_a_CPPFLAGS = @LIBBOOST_CPPFLAGS@ @MYSQL_CPPFLAGS@ 
