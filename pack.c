#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "argparse.h"
#include "arraylist.h"
#include "common.h"
#include "compress/dirwalk.h"
#include "container.h"
#include "cwalk.h"
#include "hashtable.h"
#include "lib/container.h"
#include "pack.h"
#include "threadpool.h"
#include "zstd.h"
#include "zstd_util.h"

int verbose = 0;
int argcount;

int main(int argc, const char **argv) {
    const char *const usages[] = {"pack [options] [path to other path]", NULL};
    char *archive_path = NULL;
    int num_threads = 0;
    int decompress = 0;
    int compress = 0;
    int compression_level = 0;
    long long int chunk_size = 0;
    struct cwk_segment segment;
    struct stat stbuf;
    struct argparse_option options[] = {OPT_HELP(),
                                        OPT_GROUP("BASIC OPERATIONS"),
                                        OPT_BOOLEAN('v', "verbose", &verbose, "verbose", NULL, 0, 0),
                                        OPT_INTEGER('j', "thread-num", &num_threads, "no. of threads", NULL, 0, 0),
                                        OPT_INTEGER('l', "compression-level", &compression_level, "compression level", NULL, 0, 0),
                                        OPT_INTEGER('C', "chunk-size", &chunk_size, "Chunk Size in MB (not to be used with decompress)", NULL, 0, 0),
                                        OPT_BOOLEAN('x', "decompress", &decompress, "option to decompress", NULL, 0, 0),
                                        OPT_BOOLEAN('c', "compress", &compress, "option to compress", NULL, 0, 0),
                                        OPT_STRING('f', "archive", &archive_path, "path to archive", NULL, 0, 0),
                                        OPT_END()};
    struct argparse argparse;
    argparse_init(&argparse, options, usages, 0);
    argparse_describe(&argparse, "a tool to compress files and folder FAST", "provide path to compress a file or folder or to decompress a .cpack file");
    argc = argparse_parse(&argparse, argc, argv);
    argcount = argc;
    if ((decompress && compress) || (!decompress && argc != 1 && !archive_path) || (!compress && argc != 1 && !archive_path) || (decompress && chunk_size)) {
        CHECK(0, "please provide proper arguments");
    }
    if (compress) {
        char *abs_archive_path = getcwd(NULL, PATH_MAX);
        cwk_path_get_absolute(abs_archive_path, archive_path, abs_archive_path, PATH_MAX);
        if (!num_threads) { num_threads = get_cpu_threads()->nprocs; }
        if (!compression_level) { compression_level = 3; }
        if (!chunk_size) { chunk_size = 3; }
        for (int i = 0; i < argc; i++) {
            char *ccwd = getcwd(NULL, PATH_MAX);
            cwk_path_get_absolute(ccwd, *(argv + i), ccwd, PATH_MAX);
            mt_compress_handler(ccwd, abs_archive_path, compression_level, num_threads, chunk_size);
            free(ccwd);
        }
        free(abs_archive_path);
    }
    if (decompress) {
        char *abs_archive_path = getcwd(NULL, PATH_MAX);
        cwk_path_get_absolute(abs_archive_path, archive_path, abs_archive_path, PATH_MAX);
        char *ocwd = getcwd(NULL, PATH_MAX);
        cwk_path_get_absolute(ocwd, *argv, ocwd, PATH_MAX);
        if (!num_threads) { num_threads = get_cpu_threads()->nprocs; }
        if (!chunk_size) { chunk_size = 4; }
        mt_decompress_handler(ocwd, abs_archive_path, num_threads);
        free(abs_archive_path);
        free(ocwd);
    }
    return 0;
}

// void singlethread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread) {
//     walk_ctx pctx = init_walk_ctx(f_path);
//     db_ctx_t *dbctx = init_db_ctx_c(db_path);
//     db_insert_file_ctx_t *i_file_ctx = init_db_insert_file_ctx(dbctx);
//     db_insert_chunk_ctx_t *i_chunk_ctx = init_db_insert_chunk_ctx(dbctx);
//     compress_ctx *cctx = init_compress_ctx(compression_level);
//     while (walk_next(&pctx)) {
//         if (!S_ISREG(pctx.stbuf->st_mode)) continue;
//         char *file_path = walk_get_absolute_path(&pctx, -1, 0);
//         char *rel_file_path = walk_get_absolute_path(&pctx, -1, 1);
//         if (!strcmp(file_path, db_path)) continue;
//         FILE *const f = fopen(file_path, "rb");
//         int id = db_insert_file(i_file_ctx, rel_file_path);
//         for (long long bytesLeft = pctx.stbuf->st_size; bytesLeft > 0;) {
//         unsigned long long readByteCount = fread(cctx->fBuffer, sizeof(char), cctx->fBufferSize, f);
//         size_t cSize = ZSTD_compress2(cctx->cctx, cctx->cBuffer, cctx->cBufferSize, cctx->fBuffer, readByteCount);
//         CHECK_ZSTD(cSize);
//         db_insert_chunk(i_chunk_ctx, readByteCount, id, pctx.stbuf->st_size - bytesLeft, cctx->cBuffer, cSize);
//         printf("size: %lld, offset: %lld, chunk_size: %lld, left_size: %lld, file: "
//            "[%d]%s\n",
//            pctx.stbuf->st_size, pctx.stbuf->st_size - bytesLeft, readByteCount, bytesLeft, id, rel_file_path);
//         bytesLeft -= readByteCount;
//         }
//         fclose(f);
//         free(rel_file_path);
//         free(file_path);
//     }
//     deinit_compress_ctx(cctx);
//     deinit_db_insert_chunk_ctx(i_chunk_ctx);
//     deinit_db_insert_file_ctx(i_file_ctx);
//     deinit_db_ctx_c(dbctx);
//     deinit_walk_ctx(&pctx);
// }

void mt_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread, long long int chunk_size) {
    walk_ctx pctx = init_walk_ctx(f_path);
    threadpool_t *threadpool = threadpool_t_init(num_thread);
    arraylist *hashtable = arraylist_create();
    db_ctx_t *dbctx = init_db_ctx_c(db_path);
    db_insert_chunk_size(dbctx, chunk_size);
    for (int i = 0; i < num_thread; i++) {
        hashtable_compress_value_t *thread_ctx = malloc(sizeof(hashtable_compress_value_t));
        thread_ctx->cctx = init_compress_ctx(compression_level, chunk_size);
        thread_ctx->chunktable_ctx = init_db_insert_chunk_ctx(dbctx);
        thread_ctx->filetable_ctx = init_db_insert_file_ctx(dbctx);
        hashtable_add(hashtable, &threadpool->thread_t_arr[i].thread, thread_ctx);
    }
    while (walk_next(&pctx)) {
        if (!S_ISREG(pctx.stbuf->st_mode)) continue;
        thread_compression_function_arg_t *job_arg = malloc(sizeof(thread_compression_function_arg_t));
        job_arg->file_path = walk_get_absolute_path(&pctx, -1, 0, 0);
        job_arg->rel_file_path = walk_get_absolute_path(&pctx, -1, 1, argcount>1);
        job_arg->file_size = pctx.stbuf->st_size;
        job_arg->hashtable = hashtable;
        if (contains(job_arg->file_path, db_path)) {
            free(job_arg->rel_file_path);
            free(job_arg->file_path);
            free(job_arg);
            continue;
        }
        threadpool_add_work(threadpool, mt_compression_function, job_arg);
    }
    threadpool_wait(threadpool);
    threadpool_destroy(threadpool);
    deinit_c_hashtable(hashtable);
    deinit_db_ctx_c(dbctx);
    deinit_walk_ctx(&pctx);
}

void *mt_compression_function(void *arg) {
    thread_compression_function_arg_t *function_arg = arg;
    long long int thread = pthread_self();
    hashtable_compress_value_t *local_htv = hashtable_get(function_arg->hashtable, &thread);
    FILE *const f = fopen(function_arg->file_path, "rb");
    change_path_type_to_unix(function_arg->rel_file_path);
    int id = db_insert_file(local_htv->filetable_ctx, function_arg->rel_file_path);
    if (verbose) { printf("%s\n", function_arg->rel_file_path); }
    for (long long bytesLeft = function_arg->file_size; bytesLeft > 0;) {
        unsigned long long readByteCount = fread(local_htv->cctx->fBuffer, sizeof(char), local_htv->cctx->fBufferSize, f);
        size_t cSize = ZSTD_compress2(local_htv->cctx->cctx, local_htv->cctx->cBuffer, local_htv->cctx->cBufferSize, local_htv->cctx->fBuffer, readByteCount);
        CHECK_ZSTD(cSize);
        db_insert_chunk(local_htv->chunktable_ctx, readByteCount, id, function_arg->file_size - bytesLeft, local_htv->cctx->cBuffer, cSize);
        // printf("size: %lld, offset: %lld, chunk_size: %lld, left_size: %lld, file: "
        //        "[%d]%s\t%lld\n",
        //        function_arg->file_size, function_arg->file_size - bytesLeft, readByteCount, bytesLeft, id, function_arg->rel_file_path, pthread_self());
        bytesLeft -= readByteCount;
    }
    fclose(f);
    free(function_arg->rel_file_path);
    free(function_arg->file_path);
    free(function_arg);
    return NULL;
}

// void singlethread_decompress_handler(char *d_f_path, char *db_path){
//     decompress_ctx *dctx = init_decompress_ctx();
//     db_ctx_t *dbctx = init_db_ctx_d(db_path);
//     db_filetable_walk_ctx_t *ftwalk = init_db_filetable_walk_ctx(dbctx);
//     db_chunktable_walk_ctx_t *ctwalk = init_db_chunktable_walk_ctx(dbctx);
//     char *absolute_decompress_file_path;
//     filetable_var_t *ftvar;
//     chunktable_var_t *ctvar;
//     char temp[280];
//     size_t len;
//     while((ftvar = db_get_filetable_row(ftwalk))!=NULL){
//         ctvar = db_get_chunktable_row(ctwalk, ftvar->id);
//         absolute_decompress_file_path = getcwd(NULL, PATH_MAX);
//         // printf("absolute_decompress_path: %s\n",absolute_decompress_file_path);
//         cwk_path_get_absolute(absolute_decompress_file_path, d_f_path, absolute_decompress_file_path, PATH_MAX);
//         // printf("resolved absolute_decompress_path: %s\n",absolute_decompress_file_path);
//         cwk_path_get_dirname(ftvar->filepath, &len);
//         // printf("ftvar get dirname: '%.*s'\n",len,ftvar->filepath);
//         strncpy_m(temp, ftvar->filepath, len);
//         // printf("ftvar get dirname (temp): %s\n",temp);
//         make_nested_folder(absolute_decompress_file_path, temp);
//         cwk_path_join(absolute_decompress_file_path, ftvar->filepath, absolute_decompress_file_path, PATH_MAX);
//         // printf("full file path: %s\n",absolute_decompress_file_path);
//         FILE *const f = fopen(absolute_decompress_file_path, "wb");
//         fclose(f);
//     }
// }

void mt_decompress_handler(char *d_f_path, char *db_path, int num_thread) {
    db_ctx_t *dbctx = init_db_ctx_d(db_path);
    db_filetable_walk_ctx_t *ftwalk = init_db_filetable_walk_ctx(dbctx);
    threadpool_t *threadpool = threadpool_t_init(num_thread);
    arraylist *hashtable = arraylist_create();
    char dirname[PATH_MAX];
    size_t len;
    filetable_var_t *ftvar;
    for (int i = 0; i < num_thread; i++) {
        hashtable_decompress_value_t *thread_ctx = malloc(sizeof(hashtable_compress_value_t));
        thread_ctx->chunktable_ctx = init_db_chunktable_walk_ctx(dbctx);
        thread_ctx->dctx = init_decompress_ctx(get_chunk_size(dbctx) + 2);
        hashtable_add(hashtable, &threadpool->thread_t_arr[i].thread, thread_ctx);
    }
    while ((ftvar = db_get_filetable_row(ftwalk)) != NULL) {
        thread_decompression_function_arg_t *job_arg = malloc(sizeof(thread_decompression_function_arg_t));
        char *cwd = getcwd(NULL, PATH_MAX * 2);
        if ((strlen(cwd) + strlen(d_f_path) + 5) > (PATH_MAX * 2)) { cwd = realloc(cwd, sizeof(char) * (strlen(cwd) + strlen(d_f_path) + 5)); }
        cwk_path_get_absolute(cwd, d_f_path, cwd, PATH_MAX * 2);
        cwk_path_get_dirname(ftvar->filepath, &len);
        strncpy_m(dirname, ftvar->filepath, len);
        CHECK(make_nested_folder(cwd, dirname) != 0, "make root folder");
        cwk_path_join(cwd, ftvar->filepath, cwd, PATH_MAX * 2);
        job_arg->file_path = cwd;
        job_arg->id = ftvar->id;
        job_arg->hashtable = hashtable;
        threadpool_add_work(threadpool, mt_decompression_function, job_arg);
        free(ftvar->filepath);
        free(ftvar);
    }
    threadpool_wait(threadpool);
    threadpool_destroy(threadpool);
    deinit_d_hashtable(hashtable);
    deinit_db_filetable_walk_ctx(ftwalk);
    deinit_db_ctx_d(dbctx);
}

void *mt_decompression_function(void *arg) {
    thread_decompression_function_arg_t *function_arg = arg;
    long long int thread = pthread_self();
    hashtable_decompress_value_t *localhtv = hashtable_get(function_arg->hashtable, &thread);
    sqlite3_bind_int64(localhtv->chunktable_ctx->stmt, 1, function_arg->id);
    db_chunktable_step_t *step_var;
    size_t decompressed_file_size = 0;
    size_t native_file_size = 0;
    if (verbose) { printf("%s\n", function_arg->file_path); }
    FILE *f = fopen(function_arg->file_path, "wb");
    while ((step_var = db_chunktable_step(localhtv->chunktable_ctx)) != NULL) {
        unsigned long long const rSize = ZSTD_getFrameContentSize(step_var->blob, step_var->compress_blob_size);
        CHECK(rSize != ZSTD_CONTENTSIZE_ERROR, "not compressed by zstd");
        CHECK(rSize != ZSTD_CONTENTSIZE_UNKNOWN, "unknown size");
        size_t dres = ZSTD_decompressDCtx(localhtv->dctx->dctx, localhtv->dctx->dBuffer, rSize, step_var->blob, step_var->compress_blob_size);
        CHECK_ZSTD(dres);
        decompressed_file_size += dres;
        native_file_size += step_var->read_file_size;
        fwrite(localhtv->dctx->dBuffer, sizeof(char), dres / sizeof(char), f);
    }
    CHECK(decompressed_file_size == native_file_size, "decompressed file and native file size imbalance");
    fclose(f);
    sqlite3_reset(localhtv->chunktable_ctx->stmt);
    free(function_arg->file_path);
    return NULL;
}
