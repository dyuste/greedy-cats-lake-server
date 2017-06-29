bin_PROGRAMS = yucode-console
LDADD = -L@prefix@/lib -L../../lib/server  -L../../lib/services/restful -L../../lib/database -L../../lib/xml -lyucode-server -lyucode-restful -lyucode-xml -lcurl -lyucode-database @MYSQL_LDFLAGS@ @LIBBOOST_LDFLAGS@ -lboost_system -lboost_thread -lboost_filesystem 
INCLUDES = -I$(top_srcdir)/lib  @LIBBOOST_CPPFLAGS@ 

yucode_console_SOURCES = main.cpp 
yucode_console_LDFLAGS = -static 
yucode_console_CFLAGS = @MYSQL_CPPFLAGS@ 
yucode_console_CXXFLAGS = @MYSQL_CPPFLAGS@
yucode_console_CPPFLAGS = @MYSQL_CPPFLAGS@
