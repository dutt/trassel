noinst_LTLIBRARIES = libtrassel.la
bin_PROGRAMS = test

base = $(top_srcdir)/../..
base_test = $(base)/test
base_src = $(base)/src
base_inc = $(base)/inc

INCLUDES = -I$(base_inc)

test_CXXFLAGS = @DEFINES@ @BOOST_CXXFLAGS@
test_SOURCES = $(base_test)/directed_test.cpp $(base_test)/directed_test.h \
               $(base_test)/group_test.cpp $(base_test)/group_test.h \
               $(base_test)/main.cpp
test_LDADD = libtrassel.la @BOOST_LIBS@

libtrassel_la_CXXFLAGS = @DEFINES@ @BOOST_CXXFLAGS@
libtrassel_la_SOURCES = $(base_src)/group.cpp $(base_inc)/group.h \
                  $(base_src)/messageclient.cpp $(base_inc)/messageclient.h \
                  $(base_src)/unix/timer.cpp $(base_inc)/timer.h \
                  $(base_inc)/common.h \
                  $(base_inc)/messagequeue.h \
                  $(base_inc)/typedefs.h

ACLOCAL_AMFLAGS = -I m4