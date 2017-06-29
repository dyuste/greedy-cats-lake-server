AC_INIT([greedy-cats-lake], [0.1], [david.yuste@gmail.com], [greedy-cats-lake], [http://greedycatslake.com])
AC_PREREQ([2.59])
AM_INIT_AUTOMAKE([1.10 no-define foreign subdir-objects])
AC_CONFIG_HEADERS([config.h])
AC_PROG_CXX
AC_LANG([C++])

LT_INIT
dnl AC_PROG_RANLIB

dnl
dnl mysql
dnl

AC_ARG_WITH(mysql_config,
       AC_HELP_STRING([--with-mysql_config=PATH],
                      [location of the mysql_config program]),
                  [mysql_config_prog=$withval],
                  [mysql_config_prog=mysql_config])

AC_PATH_PROG(MYSQL_CONFIG, $mysql_config_prog)

if test "x$MYSQL_CONFIG" = "x"; then
  AC_MSG_ERROR([Couldnt find mysql_config. Please verify that it is installed.])
else
  MYSQL_LDFLAGS=$($MYSQL_CONFIG --libs)
  MYSQL_CPPFLAGS=$($MYSQL_CONFIG --cflags)
fi

AC_SUBST(MYSQL_LDFLAGS)
AC_SUBST(MYSQL_CPPFLAGS)

AC_ARG_WITH(libboost,
       AC_HELP_STRING([--with-libboost=PATH],
                      [libboost installation path]),
                  [LIBBOOST_PATH=$withval])
                  
LIBBOOST_CPPFLAGS="-I$LIBBOOST_PATH/include"
LIBBOOST_LDFLAGS="-L$LIBBOOST_PATH/lib"

AC_SUBST(LIBBOOST_CPPFLAGS)
AC_SUBST(LIBBOOST_LDFLAGS)

dnl
dnl doxygen
dnl
DX_DOXYGEN_FEATURE(ON)
DX_DOT_FEATURE(ON)
DX_HTML_FEATURE(ON)
DX_CHM_FEATURE(OFF)
DX_CHI_FEATURE(OFF)
DX_MAN_FEATURE(OFF)
DX_RTF_FEATURE(OFF)
DX_XML_FEATURE(OFF)
DX_PDF_FEATURE(ON)
DX_PS_FEATURE(OFF)

DX_INIT_DOXYGEN($PACKAGE_NAME, doxygen.cfg)

AC_OUTPUT(Makefile src/Makefile src/server/Makefile src/console/Makefile src/bots-daemon/Makefile src/notification-feedback-daemon/Makefile src/reductions/Makefile lib/Makefile lib/database/Makefile lib/server/Makefile lib/services/Makefile lib/services/json/Makefile lib/services/restful/Makefile lib/services/restfulgame/Makefile lib/notifications/Makefile lib/xml/Makefile lib/external/Makefile lib/external/hiredis/Makefile lib/external/ios/apn/Makefile lib/external/jansson/Makefile lib/reductions/Makefile)

