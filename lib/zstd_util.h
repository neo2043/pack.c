#ifndef ZSTD_UTIL
#define ZSTD_UTIL

#include "zstd.h"

typedef struct {
    void *fBuffer;
    void *cBuffer;
    size_t fBufferSize;
    size_t cBufferSize;
    ZSTD_CCtx *cctx;
} compress_ctx;

compress_ctx* init_compress_ctx(int compression_level);
void deinit_compress_ctx(compress_ctx *ctx);

#endif // ZSTD_UTIL