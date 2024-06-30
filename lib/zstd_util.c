#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "zstd.h"
#include "zstd_util.h"

compress_ctx init_compress_ctx(void) {
    compress_ctx res;
    res.fBufferSize = 3 * 1024 * 1024;
    res.cBufferSize = ZSTD_compressBound(3 * 1024 * 1024);

    res.fBuffer = calloc(1, res.fBufferSize);
    res.cBuffer = calloc(1, res.cBufferSize);
    res.cctx = ZSTD_createCCtx();
    assert(res.cctx != NULL);
    return res;
}