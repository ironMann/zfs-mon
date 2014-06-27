/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include "zfsmon.h"

#include "zpool_prop.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "zfsmon_sqlite.h"
#include "zpool_util.h"


// #include "zpool.h"

// static
// void zpool_insane_string_to_numeric(const char *s_val, void *target)
// {
//     char *src = (char *)strdup(s_val);

//     // find and kill [%,x,X,' ']
//     const char *suffixes = "%%xX ";
//     char *endpos = strpbrk(src, suffixes);

//     if (NULL != endpos) {
//         endpos = '\0';
//     }

//     if (strlen(src) == 0) {
//         return;
//     }

//     if (NULL == strchr(src, '.')) { // INTEGER
//         if (strchr(src, '-') == src) { // negative
//             sscanf(src, "%" SCNu64 "", (int64_t *)target);

//         } else {
//             sscanf(src, "%" SCNu64 "", (uint64_t *)target);
//         }

//     } else { // DOUBLE
//         sscanf(src, "%lf", (double *)target);
//     }

//     free(src);
// }


// static
// int set_property(zfsmon_zpool_t *pool, const zpool_prop_t prop_name, const char *prop_val)
// {
//     int ret = 0;

//     // check size
//     if (sizeof(zfsmon_zpool_prop_description) / sizeof(zfsmon_prop_t) <= (size_t)prop_name) {
//         return 1; // unknown property
//     }

//     zfsmon_prop_t *prop = &(pool->properties.props[prop_name]);
//     zfsmon_prop_type_t p_type = zfsmon_zpool_prop_description[prop_name].type;

//     if (p_type != prop->type) {
//         printf("error: property type missmatch\n");
//         return 1;
//     }

//     switch (p_type) {
//         case string_prop:
//             if (prop->v.s) {
//                 free(prop->v.s);
//             }

//             prop->v.s = strdup(prop_val);
//             break;

//         case int_prop:
//         case uint_prop:
//         case double_prop:
//             zpool_insane_string_to_numeric(prop_val, &(prop->v));
//             break;

//         default:
//             // error
//             printf("error: unknown property type!");
//             exit(1);
//             break;
//     }

//     return ret;
// }


// int zfsmon_zpool_add_prop(zfsmon_zpool_t **pools, const char *zpool_name,
//                           const zpool_prop_t prop_name, const char *prop_val)
// {
//     zfsmon_zpool_t *pool = find_or_add_zpool(*pools, zpool_name);
//     // zfsmon_zpool_properties_t *pool_props = &(pool->properties);

//     if (0 != set_property(pool, prop_name, prop_val)) {
//         printf("warn: unknown prop: %s : %s\n", zpool_prop_to_name(prop_name), prop_val);
//         printf("warn: addind %s as a user prop\n", zpool_prop_to_name(prop_name));
//         zfsmon_zpool_add_user_prop(pools, zpool_name, zpool_prop_to_name(prop_name), prop_val);
//     }

//     return 0;
// }


// int zfsmon_zpool_add_user_prop(zfsmon_zpool_t **pools, const char *zpool_name,
//                                const char *prop_type, const char *prop_val)
// {
//     zfsmon_zpool_t *pool = find_or_add_zpool(*pools, zpool_name);
//     zfsmon_zpool_properties_t *pool_props = &(pool->properties);

//     if (NULL == pool_props->u_prop) {
//         pool_props->u_cap = 1;
//         pool_props->u_count = 0;
//         pool_props->u_prop = (zfsmon_zpool_prop_val_t *) calloc(pool_props->u_cap, sizeof(zfsmon_zpool_prop_val_t));

//     } else if (pool_props->u_cap == pool_props->u_count) {
//         pool_props->u_cap += 4;
//         pool_props->u_prop = (zfsmon_zpool_prop_val_t *) realloc(pool_props->u_prop,
//                              pool_props->u_cap * sizeof(zfsmon_zpool_prop_val_t));
//         bzero(pool_props->u_prop + pool_props->u_count, 4 * sizeof(zfsmon_zpool_prop_val_t));
//     }

//     // printf("%-40s%s\n", prop_type, prop_val);

//     pool_props->u_prop[pool_props->u_count].prop_name = strdup(prop_type);
//     pool_props->u_prop[pool_props->u_count].prop_val = strdup(prop_val);
//     pool_props->u_count++;

//     return 0;
// }

// static int prop_get_callback(zpool_handle_t *zhp, void *data)
// {
//     zfsmon_zpool_private_t *pool_private = (zfsmon_zpool_private_t *) data;
//     zprop_get_cbdata_t *cbp = &(pool_private->zprop_cb);
//     char value[MAXNAMELEN];
//     zprop_source_t srctype;
//     zprop_list_t *pl;

//     for (pl = cbp->cb_proplist; pl != NULL; pl = pl->pl_next) {
//         if (pl->pl_prop == ZPROP_INVAL &&
//                 (zpool_prop_feature(pl->pl_user_prop) ||
//                  zpool_prop_unsupported(pl->pl_user_prop))) {
//             if (zpool_prop_get_feature(zhp, pl->pl_user_prop,
//                                        value, sizeof(value)) == 0) {
//                 zfsmon_zpool_add_user_prop(pool_private->pools, zpool_get_name(zhp), pl->pl_user_prop, value);
//             }

//         } else {
//             if (zpool_get_prop_literal(zhp, pl->pl_prop, value,
//                                        sizeof(value), &srctype, B_TRUE) != 0) {
//                 continue;
//             }

//             zfsmon_zpool_add_prop(pool_private->pools, zpool_get_name(zhp), pl->pl_prop, value);
//         }
//     }

//     return (0);
// }


// int zfsmon_get_pools_properties(zfsmon_zpool_t **pools)
// {
//     int ret;
//     zfsmon_zpool_private_t pool_private = { 0 };

//     pool_private.pools = pools;

//     if ((pool_private.g_zfs = libzfs_init()) == NULL) {
//         return (1);
//     }

//     pool_private.zprop_cb.cb_first      = B_TRUE;
//     pool_private.zprop_cb.cb_sources    = ZPROP_SRC_ALL;
//     pool_private.zprop_cb.cb_columns[0] = GET_COL_NAME;
//     pool_private.zprop_cb.cb_columns[1] = GET_COL_PROPERTY;
//     pool_private.zprop_cb.cb_columns[2] = GET_COL_VALUE;
//     pool_private.zprop_cb.cb_columns[3] = GET_COL_SOURCE;
//     pool_private.zprop_cb.cb_type       = ZFS_TYPE_POOL;
//     pool_private.zprop_cb.cb_proplist   = NULL;

//     ret = for_each_pool(&pool_private.zprop_cb.cb_proplist, prop_get_callback, &pool_private);

//     zprop_free_list(pool_private.zprop_cb.cb_proplist);

//     libzfs_fini(pool_private.g_zfs);
//     return (ret);
// }


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
