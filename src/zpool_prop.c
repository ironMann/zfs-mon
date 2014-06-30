/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "zfsmon.h"
#include "zfsmon_sqlite.h"
#include "zpool_prop.h"
#include "zpool_util.h"

static
int prop_get_all(zpool_handle_t *zhp, void *data)
{
    zfsmon_t *h = (zfsmon_t *) data;
    zprop_get_cbdata_t *cbp = (zprop_get_cbdata_t *)h->t;
    char value[MAXNAMELEN];
    zprop_source_t srctype;
    zprop_list_t *pl;

    for (pl = cbp->cb_proplist; pl != NULL; pl = pl->pl_next) {
        if (pl->pl_prop == ZPROP_INVAL &&
                (zpool_prop_feature(pl->pl_user_prop) ||
                 zpool_prop_unsupported(pl->pl_user_prop))) {
            if (zpool_prop_get_feature(zhp, pl->pl_user_prop,
                                       value, sizeof(value)) == 0) {

                zfsmon_db_zpool_prop_add(h->db, zpool_get_name(zhp), pl->pl_user_prop, value);
            }

        } else {
            if (zpool_get_prop_literal(zhp, pl->pl_prop, value,
                                       sizeof(value), &srctype, B_TRUE) != 0) {
                continue;
            }

            zfsmon_db_zpool_prop_add(h->db, zpool_get_name(zhp), zpool_prop_to_name(pl->pl_prop), value);
        }
    }

    return (0);
}

int zfsmon_zpool_prop_get_all(struct zfsmon *h)
{
    int ret;

    zprop_get_cbdata_t zprop_cb = { 0 };

    zprop_cb.cb_first      = B_TRUE;
    zprop_cb.cb_sources    = ZPROP_SRC_ALL;
    zprop_cb.cb_columns[0] = GET_COL_NAME;
    zprop_cb.cb_columns[1] = GET_COL_PROPERTY;
    zprop_cb.cb_columns[2] = GET_COL_VALUE;
    zprop_cb.cb_columns[3] = GET_COL_SOURCE;
    zprop_cb.cb_type       = ZFS_TYPE_POOL;
    zprop_cb.cb_proplist   = NULL;

    h->t = &zprop_cb;

    ret = for_each_pool(&zprop_cb.cb_proplist, prop_get_all, h);

    zprop_free_list(zprop_cb.cb_proplist);
    h->t = NULL;

    return (ret);
}
