#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "zstd.h"
#include "zstd_util.h"
#include "common.h"

compress_ctx init_compress_ctx(int compression_level) {
    compress_ctx res;
    res.fBufferSize = 3 * 1024 * 1024;
    res.cBufferSize = ZSTD_compressBound(3 * 1024 * 1024);
    res.fBuffer = calloc(1, res.fBufferSize);
    res.cBuffer = calloc(1, res.cBufferSize);
    res.cctx = ZSTD_createCCtx();
    assert(res.cctx != NULL);
    CHECK_ZSTD(ZSTD_CCtx_setParameter(res.cctx, ZSTD_c_compressionLevel, compression_level));
    return res;
}