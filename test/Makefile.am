AUTOMAKE_OPTIONS=subdir-objects
noinst_PROGRAMS = testrunner

testrunner_SOURCES = \
	xtl/bitmagic_test.cpp \
	xtl/block_vector_test.cpp \
	xtl/intrusive_list_test.cpp \
	xtl/unordered_vector_map_test.cpp \
	xtl/unordered_vector_set_test.cpp \
	testrunner.cpp

testrunner_CPPFLAGS=-I$(top_srcdir)/include -I$(top_srcdir)/src/libtest
testrunner_LDFLAGS=$(top_builddir)/src/libtest/libtest.la
