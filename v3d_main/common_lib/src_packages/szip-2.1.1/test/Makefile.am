#############################################################################
##                      Setup for building programs                        ##
#############################################################################

AM_CPPFLAGS=-I$(top_srcdir)/src -I$(top_srcdir)/test

#############################################################################
##                              Testing                                    ##
#############################################################################

TESTS = gentest
check_PROGRAMS = gentest burst_szip example

burst_szip_SOURCES = burst_szip.c mcgill.c
burst_szip_LDADD = ../src/libsz.la
burst_szip_DEPENDENCIES = ../src/libsz.la

example_SOURCES = example.c
example_LDADD = ../src/libsz.la
example_DEPENDENCIES = ../src/libsz.la

gentest_SOURCES = gentest.c mcgill.c
gentest_LDADD = ../src/libsz.la
gentest_DEPENDENCIES = ../src/libsz.la

check: $(bin_PROGRAMS) 
	./burst_szip -msb 8 8 1024 16 image.8.in
	./example

#############################################################################
##                          And the cleanup                                ##
#############################################################################

DISTCLEANFILES = image.8.in
