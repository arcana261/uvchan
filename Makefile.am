ACLOCAL_AMFLAGS = -I m4 --install
AM_CPPFLAGS = -I$(top_srcdir)/src/uvchan -I$(top_srcdir)/src -I$(top_srcdir)/test
AM_CFLAGS = 
AM_LDFLAGS = 

EXTRA_DIST = \
	autogen.sh \
	CPPLINT.cfg \
	libuvchan.pc

# include libuv as dependency
AM_CFLAGS += $(LIBUV_CFLAGS)
AM_LDFLAGS += $(LIBUV_LIBS)

# targets
lib_LTLIBRARIES = libuvchan-1.la

# libuvchan-1.la
libuvchan_1_la_SOURCES = \
	src/uvchan/queue.c \
	src/uvchan/queue.h \
	src/uvchan/chan.h \
	src/uvchan/chan.c
libuvchan_1_la_LDFLAGS = $(AM_LDFLAGS) -versioninfo $(LIBVERSION)

# installation header files
# include_HEADERS = \
# 	src/context/valuemap.h \
# 	src/context/context.h \
# 	src/context/error.h

# installation pkgconfig files
pkgconfiglibdir = $(libdir)/pkgconfig
pkgconfiglib_DATA = libuvchan.pc

# tests
check_PROGRAMS = \
	test/uvchan/queue_test \
	test/uvchan/chan_test

# test/uvchan/queue_test
test_uvchan_queue_test_SOURCES = test/uvchan/queue_test.c
test_uvchan_queue_test_LDADD = $(lib_LTLIBRARIES)

# test/uvchan/uvchan_test
test_uvchan_chan_test_SOURCES = test/uvchan/chan_test.c
test_uvchan_chan_test_LDADD = $(lib_LTLIBRARIES)

# makefile includes
include make/lint.am
include make/format.am
include make/check.am
include make/sanity.am
include make/coverage.am
include make/phony.am
