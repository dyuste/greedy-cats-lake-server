bin_PROGRAMS = yucode-notification-feedback-daemon
LDADD = -L@prefix@/lib -L../../lib/server  -L../../lib/services/json -L../../lib/services/restful -L../../lib/services/restfulgame -L../../lib/database -L../../lib/xml -L../../lib/notifications -L../../lib/external/ios/apn -L../../lib/external/jansson -lyucode-restfulgame -lyucode-jsonservice -lyucode-restful -lyucode-server -lyucode-notifications -lyucode-external-ios-apn -lyucode-external-jansson -lyucode-xml -lcurl -lyucode-database @MYSQL_LDFLAGS@ @LIBBOOST_LDFLAGS@ -lboost_system -lboost_thread -lboost_filesystem -lssl -lcrypto
INCLUDES = -I$(top_srcdir)/lib  @LIBBOOST_CPPFLAGS@ 

yucode_notification_feedback_daemon_SOURCES = main.cpp
yucode_notification_feedback_daemon_LDFLAGS = -static
yucode_notification_feedback_daemon_CPPFLAGS = @MYSQL_CPPFLAGS@ 
yucode_notification_feedback_daemon_CXXFLAGS = @MYSQL_CPPFLAGS@
yucode_notification_feedback_daemon_CFLAGS = @MYSQL_CPPFLAGS@
