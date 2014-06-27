/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#include <config.h>

#include <stddef.h>
#include <string.h>
#include <syslog.h>

#include "zfsmon.h"
#include "zfsmon_sqlite.h"

#include "zpool.h"
#include "zpool_prop.h"
#include "zpool_iostat.h"

#include <libzfs.h>

/**
 * @brief Initializes zfsmon instance.
 * @details Function also performs initial update of zfs data.
 *          To cleanup use zfsmon_fini().
 *
 * @param h Pointer to preallocated instance handle structure.
 * @return On success 0.
 */
int zfsmon_init(zfsmon_t *h)
{
    int rc;

    rc = zfsmon_db_init(&h->db);
    if (rc != 0) {
        fprintf(stderr, "error: can not create sqlite database\n");
        return rc;
    }

    rc = zfsmon_update(h);
    if (rc != 0) {
        fprintf(stderr, "error: can not open libzfs\n");
        fprintf(stderr, "error: make sure zfs module is installed and you have permissions to access /dev/zfs\n");
        return rc;
    }

    return 0;
}

/**
 * @brief Closes zfsmon instance.
 *
 * @param h Instance handle.
 */
int zfsmon_fini(zfsmon_t *h)
{
    zfsmon_db_fini(h->db);
    bzero(h, sizeof(zfsmon_t));

    return 0;
}

/**
 * @brief Function refreses all data form zfs library.
 * @details Functions get current data from zfs. It should also be called periodicaly
 *          in order to estimate iostat information.
 *
 * @param h zfsmon instance handle.
 * @return 0 on sucess, -1 on permission issues, and positive integer in case of other errors.
 */
int zfsmon_update(zfsmon_t *h)
{
    int rc = 0;
    /**
     * Open/Close libzfs on every update because it leaks memory,
     * unless libzfs_fini() is called.
     */
    if ((h->zfs = libzfs_init()) == NULL) {
        return (-1);
    }

    /* get zpools */
    rc |= zfsmon_zpool_get_all(h);
    /* get zpools props */
    rc |= zfsmon_zpool_prop_get_all(h);
    /* get zpools iostats */
    rc |= zfsmon_zpool_iostat_get_all(h);

    /* close libzfs library*/
    libzfs_fini(h->zfs);
    h->zfs = NULL;

    return rc;
}


char **zfsmon_zpool_all(zfsmon_t *h)
{
    return zfsmon_db_zpool_get(h->db);
}

void zfsmon_zpool_all_free(char **pools)
{
    zfsmon_db_zpool_get_free(pools);
}


void zfsmon_zpool_prop_str(zfsmon_t *h, const char *pool_name, const char *prop_name, char *prop_value, const size_t prop_value_len)
{
    zfsmon_db_zpool_prop_get(h->db, pool_name, prop_name, prop_value, prop_value_len);
}

double zfsmon_zpool_prop_double(zfsmon_t *h, const char *pool_name, const char *prop_name)
{
    double v = -0.0;
    char buffer[32];
    zfsmon_db_zpool_prop_get(h->db, pool_name, prop_name, buffer, 32);
    sscanf(buffer, "%lf", &v);
    return v;
}

uint64_t zfsmon_zpool_prop_uint64(zfsmon_t *h, const char *pool_name, const char *prop_name)
{
    uint64_t v = 0ULL;
    char buffer[32];
    zfsmon_db_zpool_prop_get(h->db, pool_name, prop_name, buffer, 32);
    sscanf(buffer, "%" SCNu64 "", &v);
    return v;
}


int zfsmon_db_dump(zfsmon_t *h)
{
    zfsmon_db_dump_debug(h->db);
    return 0;
}

/*
 * Utility function to guarantee malloc() success.
 */
void *xmalloc(const size_t size)
{
    void *data;

    if ((data = calloc(1, size)) == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    return data;
}

inline
void *xcalloc(const size_t count, const size_t size)
{
    void *data;

    if ((data = calloc(count, size)) == NULL) {
        fprintf(stderr, "error: out of memory\n");
        exit(1);
    }

    return data;
}

inline
char *xstrdup(const char *const s)
{
    char *copy;
    size_t len = strlen(s) + 1;

    copy = xcalloc(1, len);
    memcpy(copy, s, len);

    return copy;
}
