# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
noinst_PROGRAMS += ganglia

ganglia_SOURCES = src/ganglia/ganglia.c
ganglia_LDADD = libzfsmon.la
# ganglia_CFLAGS = $(DEBUG_CFLAGS)
# ganglia_LDFLAGS = $(DEBUG_LDFLAGS)

lib_LTLIBRARIES += libzfsgmon.la

libzfsgmon_la_SOURCES = src/ganglia/zfspool_gmon.c \
                             src/ganglia/zfspool_metrics.c

libzfsgmon_la_LIBADD = libzfsmon.la

libzfsgmon_la_CFLAGS = $(AM_CFLAGS) -I/usr/include/apr-1.0
#libzfsgmon_la_LDFLAGS = $(DEBUG_LDFLAGS)
