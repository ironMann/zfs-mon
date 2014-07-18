noinst_PROGRAMS += ganglia
ganglia_SOURCES = src/ganglia/ganglia.c
ganglia_LDADD = libzfsmon.la


lib_LTLIBRARIES += libzfsgmon.la
libzfsgmon_la_SOURCES = src/ganglia/zfspool_gmon.c \
                        src/ganglia/zfspool_metrics.c src/ganglia/zfspool_metrics.h

libzfsgmon_la_LIBADD  = libzfsmon.la
libzfsgmon_la_CFLAGS  = $(AM_CFLAGS) -I/usr/include/apr-1.0
libzfsgmon_la_LDFLAGS = -no-undefined
libzfsgmon_ladir      = $(sysconfdir)/zfsmon
libzfsgmon_la_DATA    = conf/mod_zfsmon.conf


dist_bin_SCRIPTS = src/ganglia/zfs_gmetric.py
