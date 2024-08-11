#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "common.h"
#include "container.h"
#include "sqlite3.h"

db_ctx init_db_ctx(char *db_path) {
    db_ctx ctx;
    db_ctx *_ctx = &ctx;
    CHECK_SQLITE(sqlite3_open(db_path, &ctx.DB), _ctx, "db open");

    char *sqlitesetupdb = "PRAGMA foreign_keys = ON;"
                          "PRAGMA journal_mode = WAL;";

    char *sqlcreatefiletable = "CREATE TABLE IF NOT EXISTS FILE_TABLE("
                               "ID          INTEGER    NOT NULL  PRIMARY KEY  AUTOINCREMENT, "
                               "FILE_PATH   TEXT       MOT NULL  UNIQUE);";

    char *sqlcreatechunktable = "CREATE TABLE IF NOT EXISTS CHUNKS("
                                "ID       INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                                "SIZE     INTEGER NOT NULL, "
                                "FILE_ID  INTEGER NOT NULL, "
                                "OFFSET   INTEGER NOT NULL, "
                                "CHUNK    BLOB    NOT NULL, "
                                
                                "FOREIGN KEY(FILE_ID) REFERENCES FILE_TABLE(ID) ON DELETE CASCADE);";

    char *sqlcreatechunkindex = "CREATE UNIQUE INDEX IF NOT EXISTS CHUNKS_INDEX "
                                "ON CHUNKS (FILE_ID,OFFSET);";

    CHECK_SQLITE(sqlite3_exec(ctx.DB, sqlitesetupdb, NULL, 0, NULL), _ctx, "sqlite setup db");
    CHECK_SQLITE(sqlite3_exec(ctx.DB, sqlcreatefiletable, NULL, 0, NULL), _ctx, "create file table");
    CHECK_SQLITE(sqlite3_exec(ctx.DB, sqlcreatechunktable, NULL, 0, NULL), _ctx, "create chunk table");
    CHECK_SQLITE(sqlite3_exec(ctx.DB, sqlcreatechunkindex, NULL, 0, NULL), _ctx, "create chunk index");

    return ctx;
}

void deinit_db_ctx(db_ctx *ctx){
    CHECK_SQLITE(sqlite3_close(ctx->DB), ctx, "deinit db ctx");
}

db_insert_file_ctx init_db_insert_file_ctx(db_ctx *ctx) {
    db_insert_file_ctx stmt_ctx;
    stmt_ctx.DB = ctx->DB;
    char *query = "INSERT INTO FILE_TABLE (FILE_PATH) VALUES (?) RETURNING ID;";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx.stmt, NULL), ctx, "file table stmt prepare");
    return stmt_ctx;
}

void deinit_db_insert_file_ctx(db_insert_file_ctx *ctx){
    CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_insert_file_ctx deinit");
}

int db_insert_file(db_insert_file_ctx *ctx, char *fpath) {
    CHECK_SQLITE(sqlite3_bind_text(ctx->stmt, 1, fpath, strlen(fpath), NULL), ctx, "insert file table text bind");
    CHECK_SQLITE(sqlite3_step(ctx->stmt), ctx, "insert file table step");
    int id = sqlite3_column_int(ctx->stmt, 0);
    CHECK_SQLITE(sqlite3_reset(ctx->stmt), ctx, "insert in file table stmt reset");
    return id;
}

db_insert_chunk_ctx init_db_insert_chunk_ctx(db_ctx *ctx) {
    db_insert_chunk_ctx stmt_ctx;
    stmt_ctx.DB = ctx->DB;
    char *query = "INSERT INTO CHUNKS (SIZE,FILE_ID,OFFSET,CHUNK) VALUES (?,?,?,?)";
    CHECK_SQLITE(sqlite3_prepare_v2(ctx->DB, query, strlen(query), &stmt_ctx.stmt, NULL), ctx, "chunk table stmt prepare");
    return stmt_ctx;
}

void deinit_db_insert_chunk_ctx(db_insert_chunk_ctx *ctx){
    CHECK_SQLITE(sqlite3_finalize(ctx->stmt), ctx, "db_insert_chunk_ctx deinit");
}

void db_insert_chunk(db_insert_chunk_ctx *ctx, const int read_size, const int file_id, const uint64_t offset, const void *chunk, const int cSize) {
    CHECK_SQLITE(sqlite3_bind_int(ctx->stmt, 1, read_size), ctx, "insert chunk table int (size) bind");
    CHECK_SQLITE(sqlite3_bind_int(ctx->stmt, 2, file_id), ctx, "insert chunk table int (file_id) bind");
    CHECK_SQLITE(sqlite3_bind_int64(ctx->stmt, 3, offset), ctx, "insert chunk table int (offset) bind");
    CHECK_SQLITE(sqlite3_bind_blob(ctx->stmt, 4, chunk, cSize, NULL), ctx, "insert chunk table chunk bind");
    CHECK_SQLITE(sqlite3_step(ctx->stmt), ctx, "insert chunk table chunk step");
    CHECK_SQLITE(sqlite3_reset(ctx->stmt), ctx, "insert in chunk table stmt reset");
}

// char *listfiletable = "SELECT ID,FILE_PATH FROM FILE_TABLE;";

// char *getfilesize = "SELECT SUM(SIZE) FROM CHUNKS WHERE FILE_ID = ?;";

// char *getfilechunks = "SELECT ID, SIZE, OFFSET FROM CHUNKS WHERE FILE_ID = ? "
//                       "ORDER BY OFFSET ASC;";

// int callback(void *ctx, int argc, char **argv, char **columns) {
//     for (int i = 0; i < argc; i++) {
//         printf("%s : %s ", columns[i], argv[i] ? argv[i] : "NULL");
//     }
//     printf("\n");
//     return 0;
// }