dnl Autotool for XTL

AC_PREREQ(2.59)
AC_INIT([xtl], [0.1.0])
AC_CANONICAL_SYSTEM
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE

dnl ---------------------------------------------------------------------------
dnl Environment checks

AM_PROG_CC_C_O
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AC_PROG_MKDIR_P
AC_HEADER_STDC([])

dnl Always use C11
AM_CXXFLAGS="-std=c++11 -Wno-deprecated"

dnl ---------------------------------------------------------------------------
dnl Place this copyright notice in generated configure

AC_COPYRIGHT([Copyright (c) Paul Glendenning. All rights reserved.])

dnl ---------------------------------------------------------------------------
dnl Enable/disable debugging

AC_ARG_ENABLE([debug],
              [AC_HELP_STRING([--enable-debug],
                              [Build binaries with debugging info.])],
              [AS_IF([test "x$enable_debug" = "xyes"], 
                       [AM_CXXFLAGS="${AM_CXXFLAGS} -g -O0 -DDEBUG"],
                     [test "x$enable_debug" = "xno"], 
                       [AM_CXXFLAGS="${AM_CXXFLAGS} -DNDEBUG"])],
              [AM_CXXFLAGS="${AM_CXXFLAGS} -DNDEBUG"])

AM_CONDITIONAL(IS_DEBUGON, [echo "${CXXFLAGS}${AM_CXXFLAGS}" | grep 'DDEBUG' >/dev/null 2>/dev/null ])

dnl ---------------------------------------------------------------------------

AC_CONFIG_FILES(
    [
    Makefile
    include/xtl/Makefile
    src/libtest/Makefile
    test/Makefile
    ])

AC_SUBST(AM_CXXFLAGS)
AC_OUTPUT

