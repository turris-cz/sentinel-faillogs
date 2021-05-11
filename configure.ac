AC_INIT([sentinel-faillogs], [0.1.0], [tech.support@turris.cz])
AC_CONFIG_MACRO_DIRS([.m4])
AC_CONFIG_AUX_DIR([.aux])

AM_INIT_AUTOMAKE([
  foreign silent-rules subdir-objects
  -Wall -Wno-portability
  dist-zip dist-xz
])
AM_SILENT_RULES([yes])

AC_PROG_CC
AC_GNU_SOURCE

AX_CHECK_COMPILE_FLAG([-std=c11], , AC_MSG_ERROR([Compiler with C11 standard support is required]))
AX_APPEND_FLAG([-std=c11])

AC_CHECK_FILE([${0%/*}/bootstrap],[
  AC_PATH_PROG([GPERF], [gperf])
  AS_IF([test -z "$GPERF"], [AC_MSG_ERROR([Missing gperf generator])])
])
AM_CONDITIONAL([GPERF_GENERATE], [test -n "$GPERF"])


PKG_CHECK_MODULES([logc], [logc >= 0.2.0 logc_argp])
PKG_CHECK_MODULES([libevent], [libevent >= 2.0])
PKG_CHECK_MODULES([logc_event], [libevent_logc])
PKG_CHECK_MODULES([czmq], [libczmq])
PKG_CHECK_MODULES([logc_czmq], [libczmq_logc])
PKG_CHECK_MODULES([msgpack], [msgpack])
PKG_CHECK_MODULES([libconfig], [libconfig])
AC_LINK_IFELSE(
  [AC_LANG_PROGRAM([#include <argp.h>],[argp_parse(0,1,NULL,0,0,0);])],,
  [argp_standalone_required="yes"]
)
AS_IF([test "$argp_standalone_required" = "yes"],
  AC_CHECK_LIB([argp], [argp_parse], , AC_MSG_ERROR([Unable to find libargp])))

dnl Options from environment
AC_ARG_VAR([defconfig], [Default path to the configuration file])
AS_IF([test -z "${defconfig}"], [defconfig="/etc/sentinel/faillogs.cfg"])
AC_DEFINE_UNQUOTED([DEFAULT_CONFIG_FILE], ["${defconfig}"],
  [Default path to the configuration file])

AC_ARG_VAR([defsocket], [Default URI to Sentinel Proxy socket])
AS_IF([test -z "${defsocket}"], [defsocket="ipc:///tmp/sentinel_pull.sock"])
AC_DEFINE_UNQUOTED([DEFAULT_SOCKET_URI], ["${defsocket}"],
  [Default URI to Sentinel Proxy socket])

AC_ARG_VAR([deftopic], [Default topic used to submit data (you probably do not want to change this.)])
AS_IF([test -z "${deftopic}"], [deftopic="sentinel/collect/faillogs"])
AC_DEFINE_UNQUOTED([DEFAULT_TOPIC], ["${deftopic}"],
  [Default topic uised to submit data])


dnl Tests
AC_ARG_ENABLE([tests], AC_HELP_STRING([--enable-tests], [Whether to enable unit tests]))
AM_CONDITIONAL([ENABLE_TESTS], [test "x$enable_tests" = "xyes"])
AS_IF([test "x$enable_tests" = "xyes"], [
  PKG_CHECK_MODULES([CHECK], [check >= 0.11])
  AC_REQUIRE_AUX_FILE([tap-driver.sh])
])
AX_VALGRIND_CHECK

dnl Linters
AC_ARG_ENABLE([linters], AC_HELP_STRING([--enable-linters], [Whether to enable code linting support (cppcheck)]))
AM_CONDITIONAL([ENABLE_LINTERS], [test "x$enable_linters" = "xyes"])
AS_IF([test "x$enable_linters" = "xyes"], [
  AC_PATH_PROG([CPPCHECK], [cppcheck])
  AS_IF([test -z "$CPPCHECK" ], AC_MSG_ERROR([Missing linter cppcheck]))
])

dnl Coverage
AX_CODE_COVERAGE


AC_CONFIG_FILES([Makefile])

AC_OUTPUT
