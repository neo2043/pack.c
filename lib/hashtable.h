#ifndef HASHTABLE
#define HASHTABLE

#include "arraylist.h"
#include "container.h"
#include "zstd_util.h"

typedef struct {
    void *key;
    void *value;
} hashtable_t;

typedef struct {
    db_insert_file_ctx_t *filetable_ctx;
    db_insert_chunk_ctx_t *chunktable_ctx;
    compress_ctx *cctx;
} hashtable_compress_value_t;

typedef struct {
    db_chunktable_walk_ctx_t *chunktable_ctx;
    decompress_ctx *dctx;
} hashtable_decompress_value_t;

void hashtable_add(arraylist *l, void *key, void *value);
void *hashtable_get(arraylist *l, void *key);
void deinit_c_hashtable(arraylist *arr);
void deinit_d_hashtable(arraylist *arr);

#endif