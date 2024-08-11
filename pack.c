#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "common.h"
#include "compress/walk.h"
#include "container.h"
#include "cwalk.h"
#include "zstd.h"
#include "zstd_util.h"
#include "threadpool.h"

void compress(char *f_path, char *db_path);

int main() {
    char *db_path = getcwd(NULL, PATH_MAX);
    cwk_path_get_absolute(db_path, "./db/out.db", db_path, PATH_MAX);

    // compress("D:\\Games\\ELDEN RING", db_path);
    // compress(".", db_path);
    // compress("D:\\Games\\ELDEN RING", ":memory:");
    // compress("/mnt/d/Games/ELDEN RING", db_path);
    // compress("/mnt/d/Games/ELDEN RING", ":memory:");

    compress(".", ":memory:");
    return 0;
}

void compress(char *f_path, char *db_path) {
    walk_ctx pctx = init_walk_ctx(f_path);
    db_ctx dbctx = init_db_ctx(db_path);
    db_insert_file_ctx i_file_ctx = init_db_insert_file_ctx(&dbctx);
    db_insert_chunk_ctx i_chunk_ctx = init_db_insert_chunk_ctx(&dbctx);
    compress_ctx cctx = init_compress_ctx(1);
    while (walk_next(&pctx)) {
        if (!S_ISREG(pctx.stbuf->st_mode)) continue;
        char *file_path = walk_get_absolute_path(&pctx, -1, 0);
        char *rel_file_path = walk_get_absolute_path(&pctx, -1, 1);
        if (!strcmp(file_path, db_path)) continue;
        FILE *const f = fopen(file_path, "rb");
        int id = db_insert_file(&i_file_ctx, rel_file_path);
        for (long long bytesLeft = pctx.stbuf->st_size; bytesLeft > 0;) {
            unsigned long long readByteCount = fread(cctx.fBuffer, sizeof(char), cctx.fBufferSize, f);
            size_t cSize = ZSTD_compress2(cctx.cctx, cctx.cBuffer, cctx.cBufferSize, cctx.fBuffer, readByteCount);
            CHECK_ZSTD(cSize);
            db_insert_chunk(&i_chunk_ctx, readByteCount, id, pctx.stbuf->st_size - bytesLeft, cctx.cBuffer, cSize);
            printf("size: %lld, offset: %lld, chunk_size: %lld, left_size: %lld, file: "
                   "[%d]%s\n",
                   pctx.stbuf->st_size, pctx.stbuf->st_size - bytesLeft, readByteCount, bytesLeft, id, rel_file_path);
            bytesLeft -= readByteCount;
        }
        fclose(f);
        free(rel_file_path);
        free(file_path);
    }
    deinit_compress_ctx(&cctx);
    deinit_db_insert_chunk_ctx(&i_chunk_ctx);
    deinit_db_insert_file_ctx(&i_file_ctx);
    deinit_db_ctx(&dbctx);
    deinit_walk_ctx(&pctx);
}