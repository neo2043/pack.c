#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arraylist.h"
#include "common.h"
#include "compress/walk.h"
#include "container.h"
#include "cwalk.h"
#include "hashtable.h"
#include "pack.h"
#include "lib/common.h"
#include "threadpool.h"
#include "zstd.h"
#include "zstd_util.h"

int main() {
    char *db_path = getcwd(NULL, PATH_MAX);
    cwk_path_get_absolute(db_path, "./db/out.db", db_path, PATH_MAX);

    // compress_handler("D:\\Games\\ELDEN RING", db_path);
    multithread_compress_handler(".", db_path, 1, 4);
    // contains("look_for","");
    // compress_handler("D:\\Games\\ELDEN RING", "C:\\Users\\AUTO\\desktop\\out.db",1,12);
    // compress_handler("D:\\Games\\ELDEN RING", ":memory:",1,4);
    // compress_handler("/mnt/d/Games/ELDEN RING", db_path,1,4);
    // compress_handler("/mnt/d/Games/ELDEN RING", ":memory:",1,4);
    // printf("%d\n",sqlite3_threadsafe());
    return 0;
}

void singlethread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread) {
    walk_ctx pctx = init_walk_ctx(f_path);
    db_ctx *dbctx = init_db_ctx(db_path);
    db_insert_file_ctx *i_file_ctx = init_db_insert_file_ctx(dbctx);
    db_insert_chunk_ctx *i_chunk_ctx = init_db_insert_chunk_ctx(dbctx);
    compress_ctx *cctx = init_compress_ctx(compression_level);
    while (walk_next(&pctx)) {
        if (!S_ISREG(pctx.stbuf->st_mode)) continue;
        char *file_path = walk_get_absolute_path(&pctx, -1, 0);
        char *rel_file_path = walk_get_absolute_path(&pctx, -1, 1);
        if (!strcmp(file_path, db_path)) continue;
        FILE *const f = fopen(file_path, "rb");
        int id = db_insert_file(i_file_ctx, rel_file_path);
        for (long long bytesLeft = pctx.stbuf->st_size; bytesLeft > 0;) {
        unsigned long long readByteCount = fread(cctx->fBuffer, sizeof(char), cctx->fBufferSize, f);
        size_t cSize = ZSTD_compress2(cctx->cctx, cctx->cBuffer, cctx->cBufferSize, cctx->fBuffer, readByteCount);
        CHECK_ZSTD(cSize);
        db_insert_chunk(i_chunk_ctx, readByteCount, id, pctx.stbuf->st_size - bytesLeft, cctx->cBuffer, cSize);
        printf("size: %lld, offset: %lld, chunk_size: %lld, left_size: %lld, file: "
           "[%d]%s\n",
           pctx.stbuf->st_size, pctx.stbuf->st_size - bytesLeft, readByteCount, bytesLeft, id, rel_file_path);
        bytesLeft -= readByteCount;
        }
        fclose(f);
        free(rel_file_path);
        free(file_path);
    }
    deinit_compress_ctx(cctx);
    deinit_db_insert_chunk_ctx(i_chunk_ctx);
    deinit_db_insert_file_ctx(i_file_ctx);
    deinit_db_ctx(dbctx);
    deinit_walk_ctx(&pctx);
}

void multithread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread) {
    walk_ctx pctx = init_walk_ctx(f_path);
    threadpool_t *threadpool = threadpool_t_init(num_thread);
    arraylist *hashtable = arraylist_create();
    db_ctx *dbctx = init_db_ctx(db_path);
    for (int i = 0; i < num_thread; i++) {
        hashtable_value_t *thread_ctx = malloc(sizeof(hashtable_value_t));
        thread_ctx->cctx = init_compress_ctx(compression_level);
        thread_ctx->chunktable_ctx = init_db_insert_chunk_ctx(dbctx);
        thread_ctx->filetable_ctx = init_db_insert_file_ctx(dbctx);
        hashtable_add(hashtable, &threadpool->thread_t_arr[i].thread, thread_ctx);
    }
    while (walk_next(&pctx)) {
        if (!S_ISREG(pctx.stbuf->st_mode)) continue;
        thread_function_arg_t *job_arg = malloc(sizeof(thread_function_arg_t));
        job_arg->file_path = walk_get_absolute_path(&pctx, -1, 0);
        job_arg->rel_file_path = walk_get_absolute_path(&pctx, -1, 1);
        job_arg->file_size = pctx.stbuf->st_size;
        job_arg->hashtable = hashtable;
        if(contains(job_arg->file_path, db_path)){
            free(job_arg->rel_file_path);
            free(job_arg->file_path);
            free(job_arg);
            continue;
        }
        threadpool_add_work(threadpool, mt_compression_function, job_arg);
    }
    threadpool_wait(threadpool);
    threadpool_destroy(threadpool);
    deinit_hashtable(hashtable);
    deinit_walk_ctx(&pctx);
}

void *mt_compression_function(void *arg){
    thread_function_arg_t *function_arg = arg;
    long long int thread = pthread_self();
    hashtable_value_t *local_htv = hashtable_get(function_arg->hashtable, &thread);
    FILE *const f = fopen(function_arg->file_path, "rb");
    int id = db_insert_file(local_htv->filetable_ctx, function_arg->rel_file_path);
    for (long long bytesLeft = function_arg->file_size; bytesLeft > 0;) {
        unsigned long long readByteCount = fread(local_htv->cctx->fBuffer, sizeof(char), local_htv->cctx->fBufferSize, f);
        size_t cSize = ZSTD_compress2(local_htv->cctx->cctx, local_htv->cctx->cBuffer, local_htv->cctx->cBufferSize, local_htv->cctx->fBuffer, readByteCount);
        CHECK_ZSTD(cSize);
        db_insert_chunk(local_htv->chunktable_ctx, readByteCount, id, function_arg->file_size - bytesLeft, local_htv->cctx->cBuffer, cSize);
        printf("size: %lld, offset: %lld, chunk_size: %lld, left_size: %lld, file: "
               "[%d]%s\t%lld\n",
               function_arg->file_size, function_arg->file_size - bytesLeft, readByteCount, bytesLeft, id, function_arg->rel_file_path,pthread_self());
        bytesLeft -= readByteCount;
    }
    fclose(f);
    free(function_arg->rel_file_path);
    free(function_arg->file_path);
    free(function_arg);
    return NULL;
}

