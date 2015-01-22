#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([dstools],[1.0.2],[https://github.com/devkitPro/dstools/issues])
AC_CONFIG_SRCDIR([src/r4denc/r4denc.cpp])
AM_INIT_AUTOMAKE([1.10])

AC_CANONICAL_BUILD
AC_CANONICAL_HOST

AC_PROG_CC
AC_PROG_CXX

case "$host" in
   *-*-mingw*)
      LIBS="$LIBS -lws2_32"
      ;;
esac

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
