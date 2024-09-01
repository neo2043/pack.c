#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "container.h"
#include "sqlite3.h"

db_ctx_t *init_db_ctx_c(char *db_path) {
    db_ctx_t *ctx = malloc(sizeof(db_ctx_t));
    CHECK(ctx != NULL, "compression init_db_ctx_c");
    CHECK_SQLITE(sqlite3_open(db_path, &ctx->DB), ctx, "db open");

    char *sqlitesetupdb = "PRAGMA foreign_keys = ON;"
                          "PRAGMA journal_mode = WAL;";

    char *sqlcreatefiletable = "CREATE TABLE IF NOT EXISTS FILE_TABLE("
                               "ID          INTEGER    NOT NULL  PRIMARY KEY  AUTOINCREMENT, "
                               "FILE_PATH   TEXT       NOT NULL  UNIQUE);";

    char *sqlcreatechunktable = "CREATE TABLE IF NOT EXISTS CHUNKS("
                                "ID       INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                                "SIZE     INTEGER NOT NULL, "
                                "FILE_ID  INTEGER NOT NULL, "
                                "OFFSET   INTEGER NOT NULL, "
                                "CHUNK    BLOB    NOT NULL, "
                                "FOREIGN KEY(FILE_ID) REFERENCES FILE_TABLE(ID) ON DELETE CASCADE);";

    char *sqlcreatechunkindex = "CREATE UNIQUE INDEX IF NOT EXISTS CHUNKS_INDEX "
                                "ON CHUNKS (FILE_ID,OFFSET);";

    CHECK_SQLITE(sqlite3_exec(ctx->DB, sqlitesetupdb, NULL, 0, NULL), ctx, "sqlite setup db");
    CHECK_SQLITE(sqlite3_exec(ctx->DB, sqlcreatefiletable, NULL, 0, NULL), ctx, "create file table");
    CHECK_SQLITE(sqlite3_exec(ctx->DB, sqlcreatechunktable, NULL, 0, NULL), ctx, "create chunk table");
    CHECK_SQLITE(sqlite3_exec(ctx->DB, sqlcreatechunkindex, NULL, 0, NULL), ctx, "create chunk index");

    return ctx;
}

void deinit_db_ctx_c(db_ctx_t *ctx) { CHECK_SQLITE(sqlite3_close(ctx->DB), ctx, "deinit db ctx"); }

db_insert_file_ctx_t *init_db_insert_file_ctx(db_ctx_t *ctx) {
    db_insert_file_ctx_t *stmt_ctx = malloc(sizeof(db_insert_file_ctx_t));
    CHECK(stmt_ctx != NULL, "compress init_db_insert_file_ctx");
    stmt_ctx->DB = ctx->DB;
    char *query = "INSERT INTO FILE_TABLE (FILE_PATH) VALUES (?) RETURNING ID;";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx->stmt, NULL), ctx, "file table stmt prepare");
    return stmt_ctx;
}

void deinit_db_insert_file_ctx(db_insert_file_ctx_t *ctx) { CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_insert_file_ctx_t deinit"); }

int db_insert_file(db_insert_file_ctx_t *ctx, char *fpath) {
    CHECK_SQLITE(sqlite3_bind_text(ctx->stmt, 1, fpath, strlen(fpath), NULL), ctx, "insert file table text bind");
    CHECK_SQLITE(sqlite3_step(ctx->stmt), ctx, "insert file table step");
    int id = sqlite3_column_int(ctx->stmt, 0);
    CHECK_SQLITE(sqlite3_reset(ctx->stmt), ctx, "insert in file table stmt reset");
    return id;
}

db_insert_chunk_ctx_t *init_db_insert_chunk_ctx(db_ctx_t *ctx) {
    db_insert_chunk_ctx_t *stmt_ctx = malloc(sizeof(db_insert_chunk_ctx_t));
    CHECK(stmt_ctx != NULL, "compress init_db_insert_chunk_ctx");
    stmt_ctx->DB = ctx->DB;
    char *query = "INSERT INTO CHUNKS (SIZE,FILE_ID,OFFSET,CHUNK) VALUES (?,?,?,?)";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx->stmt, NULL), ctx, "chunk table stmt prepare");
    return stmt_ctx;
}

void deinit_db_insert_chunk_ctx(db_insert_chunk_ctx_t *ctx) { CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_insert_chunk_ctx_t deinit"); }

void db_insert_chunk(db_insert_chunk_ctx_t *ctx, const int read_size, const int file_id, const uint64_t offset, const void *chunk, const int cSize) {
    CHECK_SQLITE(sqlite3_bind_int(ctx->stmt, 1, read_size), ctx, "insert chunk table int (size) bind");
    CHECK_SQLITE(sqlite3_bind_int(ctx->stmt, 2, file_id), ctx, "insert chunk table int (file_id) bind");
    CHECK_SQLITE(sqlite3_bind_int64(ctx->stmt, 3, offset), ctx, "insert chunk table int (offset) bind");
    CHECK_SQLITE(sqlite3_bind_blob(ctx->stmt, 4, chunk, cSize, NULL), ctx, "insert chunk table chunk bind");
    CHECK_SQLITE(sqlite3_step(ctx->stmt), ctx, "insert chunk table chunk step");
    CHECK_SQLITE(sqlite3_reset(ctx->stmt), ctx, "insert in chunk table stmt reset");
}

db_ctx_t *init_db_ctx_d(char *db_path) {
    db_ctx_t *ctx = malloc(sizeof(db_ctx_t));
    CHECK(ctx != NULL, "decompress init_db_ctx_d");
    CHECK_SQLITE(sqlite3_open(db_path, &ctx->DB), ctx, "db open");
    return ctx;
}

void deinit_db_ctx_d(db_ctx_t *ctx) { CHECK_SQLITE(sqlite3_close(ctx->DB), ctx, "deinit db ctx"); }

db_filetable_walk_ctx_t *init_db_filetable_walk_ctx(db_ctx_t *ctx) {
    db_filetable_walk_ctx_t *stmt_ctx = malloc(sizeof(db_filetable_walk_ctx_t));
    CHECK(stmt_ctx != NULL, "decompress init_db_filetable_walk_ctx");
    stmt_ctx->DB = ctx->DB;
    char *query = "select ID,FILE_PATH from FILE_TABLE";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx->stmt, NULL), ctx, "filetable walk stmt prepare");
    return stmt_ctx;
}

void deinit_db_filetable_walk_ctx(db_filetable_walk_ctx_t *ctx) { CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_filetable_walk_ctx deinit"); }

filetable_var_t *db_get_filetable_row(db_filetable_walk_ctx_t *ctx) {
    int expr = sqlite3_step(ctx->stmt);
    filetable_var_t *var;
    switch (expr) {
        case SQLITE_DONE || SQLITE_OK:
            return NULL;
            break;
        case SQLITE_ROW:
            var = malloc(sizeof(filetable_var_t));
            CHECK(var != NULL, "decompress db_get_filetable_row");
            var->id = sqlite3_column_int64(ctx->stmt, 0);
            int len = sqlite3_column_bytes(ctx->stmt, 1);
            // printf("length: %d\n",len);
            var->filepath = malloc(sizeof(char) * (len + 1));
            memcpy(var->filepath, sqlite3_column_text(ctx->stmt, 1), len);
            var->filepath[len] = '\0';
            break;
        default:
            CHECK_SQLITE(expr, ctx, "db_get_filetable_row");
            break;
    }
    return var;
}

db_chunktable_walk_ctx_t *init_db_chunktable_walk_ctx(db_ctx_t *ctx) {
    db_chunktable_walk_ctx_t *stmt_ctx = malloc(sizeof(db_chunktable_walk_ctx_t));
    CHECK(stmt_ctx != NULL, "decompress init_db_chunktable_walk_ctx");
    stmt_ctx->DB = ctx->DB;
    char *query = "SELECT SIZE,CHUNK,LENGTH(CHUNK) FROM CHUNKS WHERE CHUNKS.FILE_ID=? ORDER BY OFFSET ASC";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx->stmt, NULL), ctx, "db_chunktable_walk_ctx_t init");
    return stmt_ctx;
}

void deinit_db_chunktable_walk_ctx(db_chunktable_walk_ctx_t *ctx) { CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_chunktable_walk_ctx deinit"); }

db_chunktable_step_t *db_chunktable_step(db_chunktable_walk_ctx_t *ctx) {
    int expr = sqlite3_step(ctx->stmt);
    db_chunktable_step_t *step_ret;
    switch (expr) {
        case SQLITE_DONE || SQLITE_OK:
            return NULL;
            break;
        case SQLITE_ROW:
            step_ret = malloc(sizeof(db_chunktable_step_t));
            CHECK(step_ret != NULL, "step_ret malloc");
            step_ret->blob = calloc(sqlite3_column_int(ctx->stmt, 2), sizeof(char));
            memcpy(step_ret->blob, sqlite3_column_blob(ctx->stmt, 1), sqlite3_column_int(ctx->stmt, 2));
            step_ret->compress_blob_size = sqlite3_column_int(ctx->stmt, 2);
            step_ret->read_file_size = sqlite3_column_int(ctx->stmt, 0);
            break;
        default:
            CHECK_SQLITE(expr, ctx, "db_chunktable_step");
            break;
    }
    return step_ret;
}