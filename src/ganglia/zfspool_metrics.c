/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include <gm_metric.h>
#include <apr_strings.h>

#include <stdlib.h>
#include <inttypes.h>

#include <zfsmon.h>

const uint32_t ZFSMON_ZPOOL_TMAX       = 60;
const char    *ZFSMON_ZPOOL_GROUP_PROP = "zfs zpool";

static
inline
char *zfsmon_zpool_metrics_name(apr_pool_t *p, const char *name, const char *metric_name)
{
    char buffer[512];
    snprintf(buffer, 512, "zpool_%s_%s", metric_name, name);
    return apr_pstrdup(p, buffer);
}

void
zfsmon_zpool_metric_reg_health(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "health");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_STRING;
    gmi->units = apr_pstrdup(p, "");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%s");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Health of the ZFS pool");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}

void
zfsmon_zpool_metric_get_health(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    zfsmon_zpool_prop_str(h, zpool_name, "health", val->str, MAX_G_STRING_SIZE);
}

void
zfsmon_zpool_metric_reg_size(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "size");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_STRING;
    gmi->units = apr_pstrdup(p, "GiB");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Size of the ZFS pool in GiB");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_size(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    uint64_t size = zfsmon_zpool_prop_uint64(h, zpool_name, "size");
    snprintf(val->str, MAX_G_STRING_SIZE, "%.3lf", (double)(size >> 10) / 1024. / 1024.);
}

void
zfsmon_zpool_metric_reg_free(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "free");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "B");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Free space of the ZFS pool in B");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_free(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = (double) zfsmon_zpool_prop_uint64(h, zpool_name, "free");
}

void
zfsmon_zpool_metric_reg_allocated(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "allocated");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "B");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Allocated space of the ZFS pool in B");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_allocated(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = (double) zfsmon_zpool_prop_uint64(h, zpool_name, "allocated");
}

void
zfsmon_zpool_metric_reg_capacity(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "capacity");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "%");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Capacity of the ZFS pool");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_capacity(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = zfsmon_zpool_prop_double(h, zpool_name, "capacity");
}

void
zfsmon_zpool_metric_reg_dedupratio(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "dedupratio");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "ratio");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Deduplication ratio of the ZFS pool");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_dedupratio(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = zfsmon_zpool_prop_double(h, zpool_name, "dedupratio");
}

void
zfsmon_zpool_metric_reg_iostat_rd(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "rdbw");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "B/s");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Read bandwidth");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_iostat_rd(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = zfsmon_zpool_iostat_bw_rd(h, zpool_name);
}

void
zfsmon_zpool_metric_reg_iostat_wr(apr_pool_t *p, const char *zpool_name, Ganglia_25metric *gmi)
{
    gmi->name = zfsmon_zpool_metrics_name(p, zpool_name, "wrbw");
    gmi->tmax = ZFSMON_ZPOOL_TMAX;
    gmi->type = GANGLIA_VALUE_DOUBLE;
    gmi->units = apr_pstrdup(p, "B/s");
    gmi->slope = apr_pstrdup(p, "both");
    gmi->fmt = apr_pstrdup(p, "%.3lf");
    gmi->msg_size = UDP_HEADER_SIZE + 8;
    gmi->desc = apr_pstrdup(p, "Write bandwidth");

    MMETRIC_INIT_METADATA(gmi, p);
    MMETRIC_ADD_METADATA(gmi, MGROUP, ZFSMON_ZPOOL_GROUP_PROP);
}
void
zfsmon_zpool_metric_get_iostat_wr(zfsmon_t *h, const char *zpool_name, g_val_t *val)
{
    val->d = zfsmon_zpool_iostat_bw_wr(h, zpool_name);
}
