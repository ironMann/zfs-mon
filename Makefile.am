AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS =
EXTRA_DIST =

AM_CFLAGS  = -std=c99
AM_CFLAGS += -I$(top_srcdir)/contrib -I$(top_srcdir)/src
AM_CFLAGS += -I/usr/include/libzfs -I/usr/include/libzfs/sys -I/usr/include/libzfs/linux -I/usr/include/libspl -DHAVE_IOCTL_IN_UNISTD_H

AM_LDFLAGS =

noinst_PROGRAMS =
bin_PROGRAMS =
lib_LTLIBRARIES =
man_MANS =
noinst_LTLIBRARIES =

include $(top_srcdir)/contrib/Makefile.am
include $(top_srcdir)/src/Makefile.am
include $(top_srcdir)/src/ganglia/Makefile.am
