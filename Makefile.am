ACLOCAL_AMFLAGS = -I m4 --install
AM_CPPFLAGS = -I$(top_srcdir)/src/uvchan -I$(top_srcdir)/src -I$(top_srcdir)/test
AM_CFLAGS = 
AM_LDFLAGS = 

EXTRA_DIST = \
	autogen.sh \
	CPPLINT.cfg \
	Doxyfile \
	libuvchan.pc \
	test/testing.h \
	src/uvchan/uvchan.h \
	test/installcheck/autogen.sh \
	test/installcheck/configure.ac \
	test/installcheck/Makefile.am \
	test/installcheck/test/main/main_test.c

# include libuv as dependency
AM_CFLAGS += $(LIBUV_CFLAGS) 
AM_LDFLAGS += $(LIBUV_LIBS)

# targets
lib_LTLIBRARIES = libuvchan-0.la

# libuvchan-0.la
libuvchan_0_la_SOURCES = \
	src/uvchan/error.h \
	src/uvchan/error.c \
	src/uvchan/queue.c \
	src/uvchan/queue.h \
	src/uvchan/chan.h \
	src/uvchan/chan.c \
	src/uvchan/select.h \
	src/uvchan/select.c
libuvchan_0_la_LDFLAGS = $(AM_LDFLAGS) -versioninfo $(LIBVERSION)

# installation header files
include_HEADERS = \
	src/uvchan/error.h \
	src/uvchan/queue.h \
	src/uvchan/chan.h \
	src/uvchan/select.h

# installation pkgconfig files
pkgconfiglibdir = $(libdir)/pkgconfig
pkgconfiglib_DATA = libuvchan.pc

# tests
check_PROGRAMS = \
	test/uvchan/error_test \
	test/uvchan/queue_test \
	test/uvchan/chan_test \
	test/uvchan/select_test

# test/uvchan/error_test
test_uvchan_error_test_SOURCES = test/uvchan/error_test.c
test_uvchan_error_test_LDADD = $(lib_LTLIBRARIES)

# test/uvchan/queue_test
test_uvchan_queue_test_SOURCES = test/uvchan/queue_test.c
test_uvchan_queue_test_LDADD = $(lib_LTLIBRARIES)

# test/uvchan/uvchan_test
test_uvchan_chan_test_SOURCES = test/uvchan/chan_test.c
test_uvchan_chan_test_LDADD = $(lib_LTLIBRARIES)

# test/uvchan/select_test
test_uvchan_select_test_SOURCES = test/uvchan/select_test.c
test_uvchan_select_test_LDADD = $(lib_LTLIBRARIES)

# makefile includes
include make/lint.am
include make/format.am
include make/check.am
include make/sanity.am
include make/coverage.am
include make/doxygen.am
include make/docs.am
include make/clean.am
include make/installcheck-lib.am
include make/phony.am
