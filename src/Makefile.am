noinst_LTLIBRARIES += libzfsmon.la

libzfsmon_la_SOURCES = src/zfsmon_sqlite.c src/zfsmon_sqlite.h \
                       src/zfsmon.c src/zfsmon.h \
                       src/zpool.c src/zpool.h \
                       src/zpool_util.c  src/zpool_util.h \
                       src/zpool_prop.c src/zpool_prop.h \
                       src/zpool_iostat.c src/zpool_iostat.h \
                       src/zpool_print.c src/zpool_print.h

libzfsmon_la_LIBADD = libsqlite3.la
