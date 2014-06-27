#ifndef ZFSMON_SQLITE_H
#define ZFSMON_SQLITE_H

#include <stdint.h>
#include "sqlite3.h"

struct zfsmon_zpool_iostat;

int zfsmon_db_init(sqlite3 **db);
int zfsmon_db_fini(sqlite3 *db);

uint64_t zfsmon_db_zpool_add(sqlite3 *db, const char *name);
void     zfsmon_db_zpool_invalidate_all(sqlite3 *db);
char**   zfsmon_db_zpool_get(sqlite3 *db);
void     zfsmon_db_zpool_get_free(char **pools);

void     zfsmon_db_zpool_prop_add(sqlite3 *db, const char *pool_name, const char *prop_name, const char *prop_value);
void     zfsmon_db_zpool_prop_get(sqlite3 *db,  const char *pool_name, const char *prop_name, char *prop_value, const size_t prop_value_len);

void     zfsmon_db_zpool_iostat_add(sqlite3 *db, const char *pool_name, struct zfsmon_zpool_iostat *s);
void     zfsmon_db_zpool_iostat_get(sqlite3 *db, const char *pool_name, struct zfsmon_zpool_iostat *p, struct zfsmon_zpool_iostat *c);

void zfsmon_db_dump_debug(sqlite3 *db);

#endif // ZFSMON_SQLITE_H
