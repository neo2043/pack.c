#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "arraylist.h"
#include "container.h"
#include "hashtable.h"
#include "zstd_util.h"


void hashtable_add(arraylist *l, void *key, void *value) {
    hashtable_t *ht = malloc(sizeof(hashtable_t));
    ht->key = key;
    ht->value = value;
    arraylist_add(l, ht);
}

void *hashtable_get(arraylist *l, void *key) {
    for (int i = 0; i < arraylist_size(l); i++) {
        hashtable_t *ht = arraylist_get(l, i);
        if (pthread_equal(*((pthread_t *)ht->key), *(pthread_t *)key)) { return ht->value; }
    }
    return NULL;
}

void deinit_c_hashtable(arraylist *arr) {
    arraylist_iterate(arr) {
        hashtable_t *ht = ctx.item;
        deinit_db_insert_file_ctx(((hashtable_compress_value_t *)ht->value)->filetable_ctx);
        deinit_db_insert_chunk_ctx(((hashtable_compress_value_t *)ht->value)->chunktable_ctx);
        deinit_compress_ctx(((hashtable_compress_value_t *)ht->value)->cctx);
    }
    arraylist_destroy(arr);
}

void deinit_d_hashtable(arraylist *arr) {
    arraylist_iterate(arr) {
        hashtable_t *ht = ctx.item;
        deinit_db_chunktable_walk_ctx(((hashtable_decompress_value_t *)ht->value)->chunktable_ctx);
        deinit_decompress_ctx(((hashtable_decompress_value_t *)ht->value)->dctx);
    }
}