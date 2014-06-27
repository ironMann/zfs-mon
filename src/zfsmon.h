/**
 * Copyright (c) 2014, Gvozden Neskovic <neskovic@compeng.uni-frankfurt.de>
 */

#ifndef ZFSMON_H
#define ZFSMON_H

#include <stddef.h>
#include <stdint.h>

struct sqlite3;
struct libzfs_handle;

#define ZFSMON_MAX_POOLS  (512)

typedef struct zfsmon {
    struct libzfs_handle *zfs;
    struct sqlite3 *db;

    // temp
    void *t;
} zfsmon_t;


int zfsmon_init(zfsmon_t *h);
int zfsmon_fini(zfsmon_t *h);
int zfsmon_update(zfsmon_t *h);


char **zfsmon_zpool_all(zfsmon_t *h);
void   zfsmon_zpool_all_free(char **pools);

void     zfsmon_zpool_prop_str(zfsmon_t *h, const char *pool_name, const char *prop_name, char *prop_value, const size_t prop_value_len);
double   zfsmon_zpool_prop_double(zfsmon_t *h, const char *pool_name, const char *prop_name);
uint64_t zfsmon_zpool_prop_uint64(zfsmon_t *h, const char *pool_name, const char *prop_name);

double zfsmon_zpool_iostat_bw_rd(zfsmon_t *h, const char *zpool_name);
double zfsmon_zpool_iostat_bw_wr(zfsmon_t *h, const char *zpool_name);


/* debug */
int zfsmon_db_dump(zfsmon_t *h);
int zfsmon_zpool_iostat_print(zfsmon_t *h);

/**
 * memory and string helpers
 */

void *xmalloc(const size_t size);
void *xcalloc(const size_t count, const size_t size);
char *xstrdup(const char *const s);

#ifndef bzero
#define bzero(d,n) memset(d,0,n)
#endif

#endif // ZFSMON_H
