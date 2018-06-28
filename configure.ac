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
AX_CHECK_CFLAG_APPEND([-std=gnu89])
AX_CHECK_CFLAG_APPEND([-Werror])
AX_CHECK_CFLAG_APPEND([-Wall])
AX_CHECK_CFLAG_APPEND([-fPIC])
AX_CHECK_CFLAG_APPEND([-Wextra])
AX_CHECK_CFLAG_APPEND([-Wno-unused-parameter])
AX_CHECK_CFLAG_APPEND([-Wstrict-prototypes])

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
PKG_CHECK_MODULES([LIBUV], [libuv >= 1.0 libuv < 2.0], [
    AC_DEFINE([LIBUV_1X], [1], [Use libuv branch 1.x])
], [
    PKG_CHECK_MODULES([LIBUV], [libuv >= 0.1 libuv < 1.0], [
        AC_DEFINE([LIBUV_0X], [1], [Use libuv branch 0.x])
        AX_RED_WARN([falling back to libuv branch 0.x])
    ])
])

# set output headerfile includedir
# AC_SUBST([includedir], [$includedir/SOMETHING])

# outputs
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([libuvchan.pc])

# generate all files
AC_OUTPUT
