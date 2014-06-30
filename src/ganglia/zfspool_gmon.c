/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include <gm_metric.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include <zfsmon.h>

#include "zfspool_metrics.h"

typedef void (*reg_metrics)(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi);
typedef void (*get_metrics)(struct zfsmon *h, const char *zpool_name, g_val_t *val);

typedef struct zfsmon_gmond_metric {
    reg_metrics reg;
    get_metrics get;
} zfsmon_gmond_metric_t;

extern mmodule zfsmon_module;
static apr_array_header_t *metric_info = NULL;

static zfsmon_gmond_metric_t zfsmon_zpool_metrics[] = {
    {
        .reg = zfsmon_zpool_metric_reg_health,
        .get = zfsmon_zpool_metric_get_health
    },
    {
        .reg = zfsmon_zpool_metric_reg_size,
        .get = zfsmon_zpool_metric_get_size
    },
    {
        .reg = zfsmon_zpool_metric_reg_free,
        .get = zfsmon_zpool_metric_get_free
    },
    {
        .reg = zfsmon_zpool_metric_reg_allocated,
        .get = zfsmon_zpool_metric_get_allocated
    },
    {
        .reg = zfsmon_zpool_metric_reg_capacity,
        .get = zfsmon_zpool_metric_get_capacity
    },
    {
        .reg = zfsmon_zpool_metric_reg_dedupratio,
        .get = zfsmon_zpool_metric_get_dedupratio
    },
    {
        .reg = zfsmon_zpool_metric_reg_iostat_rd,
        .get = zfsmon_zpool_metric_get_iostat_rd
    },
    {
        .reg = zfsmon_zpool_metric_reg_iostat_wr,
        .get = zfsmon_zpool_metric_get_iostat_wr
    }
};

/* zpool initial information, initialize on init and free on cleanup  */
static zfsmon_t zfsmon;
static char **initial_pools;
static int pools_cnt;
static time_t curr_pools_time;
static pthread_mutex_t pools_l = PTHREAD_MUTEX_INITIALIZER;


static const int ZFS_METRICS_CNT = sizeof(zfsmon_zpool_metrics) / sizeof(zfsmon_gmond_metric_t);
static const time_t ZFS_STALE_DATA = 2;

static
int zfsmon_metric_init(apr_pool_t *p)
{
    int i = 0;
    // const char *str_params = zfsmon_module.module_params;
    // apr_array_header_t *list_params = zfsmon_module.module_params_list;
    // mmparam *params;
    Ganglia_25metric *gmi;

    /* initialize the metric_info array */
    metric_info = apr_array_make(p, 1, sizeof(Ganglia_25metric));

    /* get zfs pool data */
    pthread_mutex_lock(&pools_l);
    zfsmon_init(&zfsmon);
    initial_pools = zfsmon_zpool_all(&zfsmon);

    /* populate metadata for each zfs pool */
    for (i = 0; initial_pools[i] != NULL; i++) {
        int j;

        debug_msg("info: registering metrics for zpool %s", initial_pools[i]);

        for (j = 0; j < ZFS_METRICS_CNT; j++) {
            gmi = apr_array_push(metric_info);
            zfsmon_zpool_metrics[j].reg(p, initial_pools[i], gmi);
        }

        pools_cnt++;
    }

    zfsmon_fini(&zfsmon);
    /* unlock zpool info */
    pthread_mutex_unlock(&pools_l);

    /* Add a terminator to the array and replace the empty static metric definition
    array with the dynamic array that we just created */
    gmi = apr_array_push(metric_info);
    memset(gmi, 0, sizeof(Ganglia_25metric));

    zfsmon_module.metrics_info = (Ganglia_25metric *) metric_info->elts;

    // debug_msg("zfsmon_metric_init: done");
    return 0;
}

static
void zfsmon_metric_cleanup(void)
{
    /* teardown zfs pool data */
    pthread_mutex_lock(&pools_l);

    zfsmon_zpool_all_free(initial_pools);

    if ((time_t)0 != curr_pools_time) {
        zfsmon_fini(&zfsmon);
    }

    pthread_mutex_unlock(&pools_l);

    debug_msg("zfsmon_metric_cleanup: done");
}

static
g_val_t zfsmon_metric_handler(int metric_index)
{
    g_val_t v;
    int i;
    bzero(&v, sizeof(g_val_t));

    if ((time_t)0 == curr_pools_time) {
        zfsmon_init(&zfsmon);
        curr_pools_time = time(NULL);
    }

    const int zpool_idx = metric_index / ZFS_METRICS_CNT;
    const int zpool_m_idx = metric_index % ZFS_METRICS_CNT;

    /* check if zpool is valid */
    if (zpool_idx < 0 || zpool_idx >= pools_cnt) {
        debug_msg("warning: invalid metric index %d", metric_index);
        debug_msg("warning: gmond needs to be restarted");
        return v;
    }

    pthread_mutex_lock(&pools_l);

    /* update zpool information is outdated, also initial empty info */
    if ((time(NULL) - curr_pools_time) >= ZFS_STALE_DATA) {
        zfsmon_update(&zfsmon);
        curr_pools_time = time(NULL);
    }

    /* get zpool by name from initial zpool structure.
       if zfs pools change in the system, gmond needs to be
       restarted to reflect the changes */
    const char *req_name = initial_pools[zpool_idx];

    /* check if the requested pool is valid now */
    char **current_pools;
    int is_valid = 0;
    current_pools = zfsmon_zpool_all(&zfsmon);

    for (i = 0; current_pools[i] != NULL; i++) {
        if (0 == strcmp(req_name, current_pools[i])) {
            is_valid = 1;
            break;
        }
    }
    zfsmon_zpool_all_free(current_pools);

    if (!is_valid) {
        debug_msg("warning: invalid zpool %s", req_name);
        debug_msg("warning: gmond needs to be restarted");
        pthread_mutex_unlock(&pools_l);
        return v;
    }

    debug_msg("zfsmon_metric_handler: get [%d] from pool [%d]%s", zpool_m_idx, zpool_idx, req_name);
    zfsmon_zpool_metrics[zpool_m_idx].get(&zfsmon, req_name, &v);

    pthread_mutex_unlock(&pools_l);

    return v;
}


mmodule zfsmon_module = {
    STD_MMODULE_STUFF,
    zfsmon_metric_init,
    zfsmon_metric_cleanup,
    NULL, /* Dynanically defined in zfsmon_metric_init() */
    zfsmon_metric_handler,
};
