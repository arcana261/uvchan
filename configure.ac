AC_PREREQ([2.69])
AC_INIT([libuvchan], [1.0.0], [https://github.com/arcana261/uvchan/issues])
AC_SUBST([LIBVERSION], [1:0:0])

# check file exists
# AC_CONFIG_SRCDIR([src/hello.c])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([1.14 -Wall -Werror foreign subdir-objects])

AM_PROG_AR
AC_PROG_CC
AC_LANG([C])
AX_COMPILER_CHECKS
AX_CHECK_CFLAG_APPEND([-Werror])
AX_CHECK_CFLAG_APPEND([-Wall])
AX_CHECK_CFLAG_APPEND([-fPIC])

LT_INIT

AC_PROG_INSTALL

# check availability of cpplint
AX_CPPLINT

# check availability of clang-format to format code
AX_CLANG_FORMAT

# check if we can perform unit tests
AX_UNITTEST

# check support for required compiler flags for sanity checkings
AX_SANITIZER

# Checks for existence of coverage tools and define variables for reporting coverage
AX_COVERAGE

# check availability of debian packaging tools
AX_DPKG

# resolve libuv
PKG_CHECK_MODULES([LIBUV], [libuv >= 0.10])

# set output headerfile includedir
AC_SUBST([includedir], [$includedir/context])

# outputs
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libuvchan.pc])

# generate all files
AC_OUTPUT
