#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "zstd.h"
#include "zstd_util.h"

compress_ctx *init_compress_ctx(int compression_level, long long int chunk_size) {
    compress_ctx *res = malloc(sizeof(compress_ctx));
    res->fBufferSize = chunk_size * 1024 * 1024;
    res->cBufferSize = ZSTD_compressBound(chunk_size * 1024 * 1024);
    res->fBuffer = calloc(1, res->fBufferSize);
    res->cBuffer = calloc(1, res->cBufferSize);
    res->cctx = ZSTD_createCCtx();
    assert(res->cctx != NULL);
    CHECK_ZSTD(ZSTD_CCtx_setParameter(res->cctx, ZSTD_c_compressionLevel, compression_level));
    return res;
}

void deinit_compress_ctx(compress_ctx *ctx) {
    free(ctx->cBuffer);
    free(ctx->fBuffer);
    CHECK_ZSTD(ZSTD_freeCCtx(ctx->cctx));
}

decompress_ctx *init_decompress_ctx(long long int chunk_size) {
    decompress_ctx *res = malloc(sizeof(decompress_ctx));
    res->dBufferSize = chunk_size * 1024 * 1024;
    res->dBuffer = calloc(1, res->dBufferSize);
    res->dctx = ZSTD_createDCtx();
    return res;
}

void deinit_decompress_ctx(decompress_ctx *ctx) {
    free(ctx->dBuffer);
    CHECK_ZSTD(ZSTD_freeDCtx(ctx->dctx));
}