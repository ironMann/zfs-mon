#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"

#include "zfsmon.h"
#include "zpool_iostat.h"
#include "zpool_util.h"

#include <libzfs.h>

static const char sql_create_table_zpool_t[] =
    "CREATE TABLE zpool_t (" \
    "id        INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," \
    "name      TEXT NOT NULL," \
    "valid     INTEGER," \
    "timestamp INTEGER," \
    "UNIQUE (name)" \
    ");";

static const char sql_create_table_zpool_devices_t[] =
    "CREATE TABLE zpool_devices_t (" \
    "id          INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT," \
    "zpool_id    INTEGER NOT NULL," \
    "name        TEXT NOT NULL," \
    "FOREIGN KEY (zpool_id) REFERENCES zpool_t(id) ON DELETE CASCADE" \
    ");";

static const char sql_create_table_zpool_props_t[] =
    "CREATE TABLE zpool_props_t (" \
    "zpool_id   INTEGER NOT NULL," \
    "name       TEXT," \
    "value      TEXT," \
    "UNIQUE (zpool_id, name)," \
    "FOREIGN KEY (zpool_id) REFERENCES zpool_t(id) ON DELETE CASCADE" \
    ");";

static const char sql_create_table_zpool_iostat_t[] =
    "CREATE TABLE zpool_iostat_t (" \
    "zpool_id   INTEGER NOT NULL," \
    "timestamp  INTEGER," \
    "iostat     BLOB," \
    "UNIQUE (zpool_id, timestamp)," \
    "FOREIGN KEY (zpool_id) REFERENCES zpool_t(id) ON DELETE CASCADE" \
    ");";

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
    int i;
    char buffer[256];

    for (i = 0; i < argc; i++) {
        sprintf(buffer, "%s = %s", azColName[i], argv[i] ? argv[i] : "NULL");
        printf("%-30s", buffer);
    }

    printf("\n");
    return 0;
}

static
void zfsmon_db_commit(sqlite3 *db)
{
    sqlite3_exec(db, "COMMIT", NULL, NULL, NULL);
}

uint64_t zfsmon_db_zpool_get_id(sqlite3 *db, const char *name)
{
    uint64_t id = 0;
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db, "select id from zpool_t where name = ?1", -1, &stmt, NULL);

    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);

    if (SQLITE_ROW == sqlite3_step(stmt)) {
        id = (uint64_t)sqlite3_column_int64(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return id;
}

uint64_t zfsmon_db_zpool_add(sqlite3 *db, const char *name)
{
    sqlite3_stmt *u_stmt = NULL, *i_stmt = NULL;

    /* be careful not to remove */
    static const char *insert_stmt = "INSERT OR IGNORE INTO zpool_t (name) VALUES (?1)";
    static const char *update_stmt = "UPDATE zpool_t " \
                                     "SET valid = 1, timestamp = strftime('%s', 'now') " \
                                     "WHERE name = ?1";

    sqlite3_prepare_v2(db, insert_stmt, -1, &i_stmt, NULL);
    sqlite3_bind_text(i_stmt, 1, name, -1, SQLITE_TRANSIENT);

    sqlite3_prepare_v2(db, update_stmt, -1, &u_stmt, NULL);
    sqlite3_bind_text(u_stmt, 1, name, -1, SQLITE_TRANSIENT);

    sqlite3_step(i_stmt);
    sqlite3_step(u_stmt);

    sqlite3_finalize(i_stmt);
    sqlite3_finalize(u_stmt);

    zfsmon_db_commit(db);

    return zfsmon_db_zpool_get_id(db, name);
}

char **zfsmon_db_zpool_get(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    size_t i = 0;
    char **pools;

    sqlite3_prepare_v2(db, "SELECT name FROM zpool_t WHERE valid = 1 ORDER BY name ASC", -1, &stmt, NULL);

    pools = (char **) xcalloc(ZFSMON_MAX_POOLS, sizeof(char *));

    while ((SQLITE_ROW == sqlite3_step(stmt)) && (i < ZFSMON_MAX_POOLS)) {
        pools[i++] = xstrdup((const char *)sqlite3_column_text(stmt, 0));
    }

    sqlite3_finalize(stmt);

    return pools;
}

void zfsmon_db_zpool_get_free(char **pools)
{
    size_t i = 0;

    while ((pools[i] != NULL) && (i < ZFSMON_MAX_POOLS)) {
        free(pools[i]);
        i++;
    }

    free(pools);
}

void zfsmon_db_zpool_invalidate_all(sqlite3 *db)
{
    sqlite3_stmt *stmt;

    sqlite3_prepare_v2(db,
                       "UPDATE zpool_t set valid = 0, timestamp = strftime('%s', 'now') " \
                       "WHERE valid = 1",
                       -1, &stmt, NULL);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    zfsmon_db_commit(db);
}

void zfsmon_db_zpool_prop_add(sqlite3 *db, const char *pool_name, const char *prop_name, const char *prop_value)
{
    sqlite3_stmt *stmt;

    uint64_t p_id = zfsmon_db_zpool_add(db, pool_name);

    sqlite3_prepare_v2(db,
                       "INSERT OR REPLACE INTO zpool_props_t (zpool_id, name, value) " \
                       "VALUES (?1, ?2, ?3)",
                       -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, p_id);
    sqlite3_bind_text(stmt, 2, prop_name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, prop_value, -1, SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    zfsmon_db_commit(db);
}

void zfsmon_db_zpool_prop_get(sqlite3 *db,  const char *pool_name, const char *prop_name, char *prop_value, const size_t prop_value_len)
{
    sqlite3_stmt *stmt;

    uint64_t p_id = zfsmon_db_zpool_add(db, pool_name);

    sqlite3_prepare_v2(db, "SELECT value FROM zpool_props_t WHERE zpool_id = ?1 AND name = ?2", -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, p_id);
    sqlite3_bind_text(stmt, 2, prop_name, -1, SQLITE_TRANSIENT);

    if (SQLITE_ROW == sqlite3_step(stmt)) {
        snprintf(prop_value, prop_value_len, "%s", (const char *)sqlite3_column_text(stmt, 0));

    } else {
        bzero(prop_value, prop_value_len);
    }

    sqlite3_finalize(stmt);
}

void zfsmon_db_zpool_iostat_add(struct sqlite3 *db, const char *pool_name, struct zfsmon_zpool_iostat *s)
{
    sqlite3_stmt *stmt;

    uint64_t p_id = zfsmon_db_zpool_add(db, pool_name);

    sqlite3_prepare_v2(db,
                       "INSERT OR REPLACE INTO zpool_iostat_t (zpool_id, timestamp, iostat) " \
                       "VALUES (?1, strftime('%s', 'now'), ?2)",
                       -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, p_id);
    sqlite3_bind_blob(stmt, 2, s, sizeof(struct zfsmon_zpool_iostat), SQLITE_TRANSIENT);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    /* trim */
    sqlite3_prepare_v2(db,
                       "DELETE FROM zpool_iostat_t " \
                       "WHERE zpool_id = ?1 AND timestamp NOT IN " \
                       "(SELECT timestamp FROM zpool_iostat_t WHERE zpool_id = ?2 " \
                       "ORDER BY timestamp DESC LIMIT 2)",
                       -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, p_id);
    sqlite3_bind_int64(stmt, 2, p_id);

    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    zfsmon_db_commit(db);
}

void zfsmon_db_zpool_iostat_get(struct sqlite3 *db, const char *pool_name,
                                struct zfsmon_zpool_iostat *p,
                                struct zfsmon_zpool_iostat *c)
{
    sqlite3_stmt *stmt;
    const char *blob;

    uint64_t p_id = zfsmon_db_zpool_get_id(db, pool_name);

    sqlite3_prepare_v2(db, "SELECT iostat FROM zpool_iostat_t WHERE zpool_id = ?1 ORDER BY timestamp DESC",
                       -1, &stmt, NULL);

    sqlite3_bind_int64(stmt, 1, p_id);

    if (c != NULL) {
        bzero(c, sizeof(struct zfsmon_zpool_iostat));
    }

    if (p != NULL) {
        bzero(p, sizeof(struct zfsmon_zpool_iostat));
    }

    if (c && SQLITE_ROW == sqlite3_step(stmt)) {
        blob = sqlite3_column_blob(stmt, 0);

        if (sqlite3_column_bytes(stmt, 0) != sizeof(struct zfsmon_zpool_iostat)) {
            fprintf(stderr, "error: iostat blob size missmatch");
            exit(1);
        }

        memcpy(c, blob, sizeof(struct zfsmon_zpool_iostat));

        if (p && SQLITE_ROW == sqlite3_step(stmt)) {
            blob = sqlite3_column_blob(stmt, 0);

            if (sqlite3_column_bytes(stmt, 0) != sizeof(struct zfsmon_zpool_iostat)) {
                fprintf(stderr, "error: iostat blob size missmatch");
                exit(1);
            }

            memcpy(p, blob, sizeof(struct zfsmon_zpool_iostat));
        }
    }

    sqlite3_finalize(stmt);
}

void zfsmon_db_dump_debug(sqlite3 *db)
{
    printf("zpool_t\n--------------------------------\n");
    sqlite3_exec(db, "select * from zpool_t", callback, 0, NULL);
    printf("\n");

    printf("zpool_props_t\n--------------------------------\n");
    sqlite3_exec(db, "select * from zpool_props_t", callback, 0, NULL);
    printf("\n");

    printf("zpool_iostat_t\n--------------------------------\n");
    sqlite3_exec(db, "select * from zpool_iostat_t ORDER BY timestamp DESC", callback, 0, NULL);
    printf("\n");
}

int zfsmon_db_init(sqlite3 **zfsdb)
{
    char *zErrMsg = 0;
    int  rc;

    /* Open database */
    // rc = sqlite3_open("/tmp/db.sqlite3", zfsdb);
    rc = sqlite3_open(":memory:", zfsdb);

    sqlite3 *db = *zfsdb;

    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        exit(0);

    } else {
        // fprintf(stdout, "Opened database successfully\n");
    }

    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", NULL, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        sqlite3_free(zErrMsg);
    }

    /* Execute SQL create table statement */
    rc = sqlite3_exec(db, sql_create_table_zpool_t, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);

    } else {
        // fprintf(stdout, "Table created successfully\n");
    }

    /* Execute SQL create table statement */
    rc = sqlite3_exec(db, sql_create_table_zpool_devices_t, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);

    } else {
        // fprintf(stdout, "Table created successfully\n");
    }

    /* Execute SQL create table statement */
    rc = sqlite3_exec(db, sql_create_table_zpool_props_t, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);

    } else {
        // fprintf(stdout, "Table created successfully\n");
    }

    /* Execute SQL create table statement */
    rc = sqlite3_exec(db, sql_create_table_zpool_iostat_t, callback, 0, &zErrMsg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
        sqlite3_free(zErrMsg);

    } else {
        // fprintf(stdout, "Table created successfully\n");
    }

    // zfsmon_db_test(db);

    return 0;
}

int zfsmon_db_fini(sqlite3 *zfsdb)
{
    sqlite3_close(zfsdb);
    return 0;
}
