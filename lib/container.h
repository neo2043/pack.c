#ifndef STORAGE_CONTAINER
#define STORAGE_CONTAINER

#include "sqlite3.h"
#include <stdint.h>

typedef struct {
    sqlite3 *DB;
} db_ctx_t;
db_ctx_t *init_db_ctx_c(char *path);
void deinit_db_ctx_c(db_ctx_t *ctx);
db_ctx_t *init_db_ctx_d(char *db_path);
void deinit_db_ctx_d(db_ctx_t *ctx);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_insert_file_ctx_t;
db_insert_file_ctx_t *init_db_insert_file_ctx(db_ctx_t *ctx);
void deinit_db_insert_file_ctx(db_insert_file_ctx_t *ctx);
int db_insert_file(db_insert_file_ctx_t *ctx, char *fpath);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_insert_chunk_ctx_t;
db_insert_chunk_ctx_t *init_db_insert_chunk_ctx(db_ctx_t *ctx);
void deinit_db_insert_chunk_ctx(db_insert_chunk_ctx_t *ctx);
void db_insert_chunk(db_insert_chunk_ctx_t *ctx, const int read_size, const int file_id, const uint64_t offset, const void *chunk, const int cSize);

void db_insert_chunk_size(db_ctx_t *ctx, long long int chunk_size);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_filetable_walk_ctx_t;
db_filetable_walk_ctx_t *init_db_filetable_walk_ctx(db_ctx_t *ctx);
void deinit_db_filetable_walk_ctx(db_filetable_walk_ctx_t *ctx);

typedef struct {
    char *filepath;
    unsigned long long int id;
} filetable_var_t;
filetable_var_t *db_get_filetable_row(db_filetable_walk_ctx_t *ctx);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_get_chunktable_row_count_t;
int db_get_chunktable_row_count(sqlite3 *db, unsigned long long id);

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *stmt;
} db_chunktable_walk_ctx_t;
db_chunktable_walk_ctx_t *init_db_chunktable_walk_ctx(db_ctx_t *ctx);
void deinit_db_chunktable_walk_ctx(db_chunktable_walk_ctx_t *ctx);

typedef struct {
    char *blob;
    unsigned long long read_file_size;
    unsigned long long compress_blob_size;
} db_chunktable_step_t;
db_chunktable_step_t *db_chunktable_step(db_chunktable_walk_ctx_t *ctx);

long long int get_chunk_size(db_ctx_t *ctx);
void list_file_table(db_ctx_t *ctx);
#endif // STORAGE_CONTAINER