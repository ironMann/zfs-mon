/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#ifndef ZMON_ZPOOL_IOSTAT_H
#define ZMON_ZPOOL_IOSTAT_H

#include <libzfs.h>

/* current iostat values */
typedef struct zfsmon_zpool_iostat
{
    vdev_stat_t iostat;
} zfsmon_zpool_iostat_t;

struct zfsmon;
int zfsmon_zpool_iostat_get_all(struct zfsmon *h);


#endif /* ZMON_ZPOOL_IOSTAT_H */
