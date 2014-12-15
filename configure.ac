
AC_INIT([gpib-server],[1.3.3],[kamae@vlsi.kuee.kyoto-u.ac.jp])
AC_CONFIG_SRCDIR([config.hin])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([config.h:config.hin])
AM_INIT_AUTOMAKE

AC_ARG_ENABLE(sicl32, [  --enable-sicl32 turn on to use Agilent sicl32 library],,enable_sicl32=yes)
if test x$enable_sicl32 = xyes; then
	AC_DEFINE(ENABLE_SICL32, , sicl32)
	[LIBS="$LIBS agilent/sicl32.lib"]
fi
AM_CONDITIONAL(SICL32, test x$enable_sicl32 = xyes)

AC_ARG_ENABLE(serial, [  --enable-serial turn on to use Windows serial port],,enable_serial=yes)
if test x$enable_serial = xyes; then
	AC_DEFINE(ENABLE_SERIAL, , serial)
fi
AM_CONDITIONAL(SERIAL, test x$enable_serial = xyes)

AC_PROG_CC
AC_PROG_CXX

AC_CHECK_HEADER(winsock2.h, [AC_DEFINE([HAVE_WINSOCKET], [], [winsock]) LIBS="$LIBS -lwsock32"])

AC_OUTPUT([Makefile])
