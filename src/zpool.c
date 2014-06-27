/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include "zfsmon.h"
#include "zfsmon_sqlite.h"
#include "zpool.h"

#include <libzfs.h>

static int
zfsmon_add_pool(zpool_handle_t *zhp, void *data)
{
    zfsmon_t *h = (zfsmon_t *) data;

    zfsmon_db_zpool_add(h->db, zpool_get_name(zhp));

    zpool_close(zhp);
    return 0;
}
int zfsmon_zpool_get_all(struct zfsmon *h)
{
    int rc;
    zfsmon_db_zpool_invalidate_all(h->db);

    rc = zpool_iter(h->zfs, zfsmon_add_pool, h);

    return rc;
}
