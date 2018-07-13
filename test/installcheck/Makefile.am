ACLOCAL_AMFLAGS = -I ../../m4 --install
AM_CPPFLAGS = -I$(top_srcdir)/../../test
AM_CFLAGS = 
AM_LDFLAGS = 

EXTRA_DIST = \
	autogen.sh

# include libuvchan as dependency
AM_CFLAGS += $(LIBUVCHAN_CFLAGS) 
AM_LDFLAGS += $(LIBUVCHAN_LIBS)

# tests
check_PROGRAMS = \
	test/main/main_test

# test/main/main_test
test_main_main_test_SOURCES = test/main/main_test.c
test_main_main_test_LDADD = $(lib_LTLIBRARIES)

# makefile includes
include ../../make/check.am
include ../../make/phony.am
