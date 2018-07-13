AC_PREREQ([2.69])
AC_INIT([libuvchan_installcheck], [0.1.0], [https://github.com/arcana261/uvchan/issues])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([../../m4])

AM_INIT_AUTOMAKE([1.14 -Wall -Werror foreign subdir-objects])

AM_PROG_AR
AC_PROG_CC
AC_LANG([C])
AX_COMPILER_CHECKS
AX_CHECK_CFLAG_APPEND([-std=gnu89])
AX_CHECK_CFLAG_APPEND([-Werror])
AX_CHECK_CFLAG_APPEND([-Wall])
AX_CHECK_CFLAG_APPEND([-fPIC])

LT_INIT

AC_PROG_INSTALL

# check if we can perform unit tests
AX_UNITTEST

# resolve our library
PKG_CHECK_MODULES([LIBUVCHAN], [libuvchan])

# enable some more compiler warnings
# these warning might mess-up with other AC_LINK_IFELSE
# and AC_RUN_IFELSE directives
AX_CHECK_CFLAG_APPEND([-Wextra])
AX_CHECK_CFLAG_APPEND([-Wno-unused-parameter])
AX_CHECK_CFLAG_APPEND([-Wstrict-prototypes])

# outputs
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile])

# generate all files
AC_OUTPUT
