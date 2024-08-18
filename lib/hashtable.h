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
    db_insert_file_ctx *filetable_ctx;
    db_insert_chunk_ctx *chunktable_ctx;
    compress_ctx *cctx;
} hashtable_value_t;

void hashtable_add(arraylist *l, void *key, void *value);
void *hashtable_get(arraylist *l, void *key);
void deinit_hashtable(arraylist *arr);

#endif