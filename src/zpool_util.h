/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#ifndef ZFSMON_ZPOOL_UTIL_H
#define ZFSMON_ZPOOL_UTIL_H

#include <libzfs.h>

int for_each_pool(zprop_list_t **proplist, zpool_iter_f func, void *data);

#endif /* ZFSMON_ZPOOL_UTIL_H */
