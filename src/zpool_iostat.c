/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */
#include <config.h>

#include <string.h>
#include <stddef.h>

#include <libuutil.h>
#include <libnvpair.h>

#include <libzfs.h>

#include "zfsmon.h"
#include "zfsmon_sqlite.h"

#include "zpool.h"
#include "zpool_iostat.h"

#include "zpool_util.h"

static int
refresh_iostat(zpool_handle_t *zhp, void *data)
{
    (void) data;
    boolean_t missing;

    if (zpool_refresh_stats(zhp, &missing) != 0) {
        return (-1);
    }

    if (missing) {
        return 1;
    }

    zpool_close(zhp);
    return (0);
}

static void
print_one_stat(uint64_t value)
{
    char buf[64];

    zfs_nicenum(value, buf, sizeof(buf));
    (void) printf("  %5s", buf);
}


static
int print_iostat(const char *pool_name, zfsmon_t *h)
{
    zfsmon_zpool_iostat_t p_stat, c_stat;
    uint64_t tdelta;
    double scale;

    zfsmon_db_zpool_iostat_get(h->db, pool_name, &p_stat, &c_stat);

    tdelta = c_stat.iostat.vs_timestamp - p_stat.iostat.vs_timestamp;
    scale = tdelta == 0 ? 1.0 : (double)NANOSEC / (double)(tdelta);

    print_one_stat((uint64_t)(scale * (c_stat.iostat.vs_ops[ZIO_TYPE_READ] - p_stat.iostat.vs_ops[ZIO_TYPE_READ])));
    print_one_stat((uint64_t)(scale * (c_stat.iostat.vs_ops[ZIO_TYPE_WRITE] - p_stat.iostat.vs_ops[ZIO_TYPE_WRITE])));
    print_one_stat((uint64_t)(scale * (c_stat.iostat.vs_bytes[ZIO_TYPE_READ] - p_stat.iostat.vs_bytes[ZIO_TYPE_READ])));
    print_one_stat((uint64_t)(scale * (c_stat.iostat.vs_bytes[ZIO_TYPE_WRITE] - p_stat.iostat.vs_bytes[ZIO_TYPE_WRITE])));
    printf("\n");

    return 0;
}

int zfsmon_zpool_iostat_print(struct zfsmon *h)
{
    print_iostat("t1", h);
    print_iostat("t2", h);
    printf("\n");
    return 0;
}

static
int print_iostat_total(zpool_handle_t *zhp, void *d)
{
    zfsmon_zpool_iostat_t stat;
    zfsmon_t *h = (zfsmon_t *)d;

    zfsmon_db_zpool_iostat_get(h->db, zpool_get_name(zhp), NULL, &stat);

    print_one_stat(stat.iostat.vs_ops[ZIO_TYPE_READ]);
    print_one_stat(stat.iostat.vs_ops[ZIO_TYPE_WRITE]);
    print_one_stat(stat.iostat.vs_bytes[ZIO_TYPE_READ]);
    print_one_stat(stat.iostat.vs_bytes[ZIO_TYPE_WRITE]);
    printf("\n");

    zpool_close(zhp);
    return 0;
}

int zfsmon_zpool_iostat_print_total(struct zfsmon *h)
{
    zpool_iter(h->zfs, print_iostat_total, h);
    return 0;
}

static
int save_iostat(zpool_handle_t *zhp, void *d)
{
    // nvlist_t **oldchild, **newchild;
    uint_t c;
    vdev_stat_t *newvs;
    nvlist_t *newnvroot;
    nvlist_t *newconfig;
    zfsmon_zpool_iostat_t iostats;
    zfsmon_t *h = (zfsmon_t *)d;

    newconfig = zpool_get_config(zhp, NULL);

    verify(nvlist_lookup_nvlist(newconfig, ZPOOL_CONFIG_VDEV_TREE, &newnvroot) == 0);
    verify(nvlist_lookup_uint64_array(newnvroot, ZPOOL_CONFIG_VDEV_STATS, (uint64_t **)&newvs, &c) == 0);

    iostats.iostat = *newvs;

    zfsmon_db_zpool_iostat_add(h->db, zpool_get_name(zhp), &iostats);

    zpool_close(zhp);
    return 0;
}

int zfsmon_zpool_iostat_get_all(struct zfsmon *h)
{
    zpool_iter(h->zfs, refresh_iostat, NULL);
    zpool_iter(h->zfs, save_iostat, h);

    return (0);
}

double zfsmon_zpool_iostat_bw_rd(struct zfsmon *h, const char *zpool_name)
{
    zfsmon_zpool_iostat_t p_stat, c_stat;
    double rd_bw = 0.;
    uint64_t tdelta;
    double scale;

    zfsmon_db_zpool_iostat_get(h->db, zpool_name, &p_stat, &c_stat);

    // in nsec
    tdelta = c_stat.iostat.vs_timestamp - p_stat.iostat.vs_timestamp;
    scale = (tdelta == 0ULL) ? 1.0 : (double)NANOSEC / (double)(tdelta);

    rd_bw = (scale * (c_stat.iostat.vs_bytes[ZIO_TYPE_READ] - p_stat.iostat.vs_bytes[ZIO_TYPE_READ]));

    if (rd_bw  < 1.0) {
        rd_bw = 0.0;
    }

    return rd_bw;
}

double zfsmon_zpool_iostat_bw_wr(struct zfsmon *h, const char *zpool_name)
{
    double wr_bw = 0.;
    zfsmon_zpool_iostat_t p_stat, c_stat;
    uint64_t tdelta;
    double scale;

    zfsmon_db_zpool_iostat_get(h->db, zpool_name, &p_stat, &c_stat);

    // in nsec
    tdelta = c_stat.iostat.vs_timestamp - p_stat.iostat.vs_timestamp;
    scale = (tdelta == 0ULL) ? 1.0 : (double)NANOSEC / (double)(tdelta);

    wr_bw = (scale * (c_stat.iostat.vs_bytes[ZIO_TYPE_WRITE] - p_stat.iostat.vs_bytes[ZIO_TYPE_WRITE]));

    if (wr_bw < 1.0) {
        wr_bw = 0.0;
    }

    return wr_bw;
}
