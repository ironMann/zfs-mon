#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])

AC_INIT(zfs-mon, 0.1, neskovic@compeng.uni-frankfurt.de)
AC_CONFIG_SRCDIR([src/zpool_util.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE
PKG_PROG_PKG_CONFIG
LT_PREREQ([2.2])
LT_INIT([dlopen disable-static])


# Checks for programs.
AC_PROG_INSTALL
AC_PROG_CC
AC_PROG_LIBTOOL

# Other tools used in this package's various make scripts
AC_PATH_PROG([TAR], [tar gtar])   # tar
AC_PATH_PROG([XARGS], [xargs])    # xargs
AC_PATH_PROG([CHMOD], [chmod])    # chmod
AC_PATH_PROG([CAT], [cat])        # cat
AC_PATH_PROG([ECHO], [echo])      # echo
AC_PATH_PROG([CP], [cp])          # cp
AC_PATH_PROG([DATE], [date])      # date
AC_PATH_PROG([MV], [mv])          # mv
AC_PATH_PROG([TEST], [test])
AC_PATH_PROG([DEBUILD], [dpkg-buildpackage], [notfound])

AC_LANG([C])


# Checks for libraries.
AC_CHECK_LIB([zfs],[libzfs_init])
AC_CHECK_LIB([zpool],[libzfs_init])
AC_CHECK_LIB([uutil],[libzfs_init])
AC_CHECK_LIB([ganglia],[libzfs_init])
AC_CHECK_LIB([apr-1],[libzfs_init])

# AC_LIB_LINKFLAGS(zfs)

# Checks for header files.
AC_CHECK_HEADERS([ganglia.h])
AC_CHECK_HEADERS([apr-1/apr.h apr-1/apu.h expat.h])
AC_HEADER_STDC

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

# Checks for library functions.

AC_CONFIG_FILES([Makefile])

AC_OUTPUT

#PKG_CHECK_MODULES([DEPS], [cairo], [], [AC_MSG_ERROR([Missing Cairo library])])
