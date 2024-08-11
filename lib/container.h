#ifndef CONTAINER
#define CONTAINER

#include <stdint.h>
#include "sqlite3.h"

typedef struct {
    sqlite3 *DB;
} db_ctx;

db_ctx init_db_ctx(char *path);
void deinit_db_ctx(db_ctx *ctx);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_insert_file_ctx;

db_insert_file_ctx init_db_insert_file_ctx(db_ctx *ctx);
void deinit_db_insert_file_ctx(db_insert_file_ctx *ctx);
int db_insert_file(db_insert_file_ctx *ctx, char *fpath);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_insert_chunk_ctx;

db_insert_chunk_ctx init_db_insert_chunk_ctx(db_ctx *ctx);
void deinit_db_insert_chunk_ctx(db_insert_chunk_ctx *ctx);
void db_insert_chunk(db_insert_chunk_ctx *ctx, const int read_size, const int file_id, const uint64_t offset, const void *chunk, const int cSize);

#endif // CONTAINER
