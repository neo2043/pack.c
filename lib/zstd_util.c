#include <assert.h>
#include <stdlib.h>

#include "common.h"
#include "zstd.h"
#include "zstd_util.h"


compress_ctx* init_compress_ctx(int compression_level) {
    compress_ctx *res = malloc(sizeof(compress_ctx));
    res->fBufferSize = 3 * 1024 * 1024;
    res->cBufferSize = ZSTD_compressBound(3 * 1024 * 1024);
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