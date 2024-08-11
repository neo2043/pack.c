#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>

#include "sqlite3.h"

#define CHECK(cond, ...)                                                                                                                                       \
    do {                                                                                                                                                       \
        if (!(cond)) {                                                                                                                                         \
            fprintf(stderr, "%s:%d CHECK(%s) failed: ", __FILE__, __LINE__, #cond);                                                                            \
            fprintf(stderr, "" __VA_ARGS__);                                                                                                                   \
            fprintf(stderr, "\n");                                                                                                                             \
            exit(1);                                                                                                                                           \
        }                                                                                                                                                      \
    } while (0)

#define CHECK_ZSTD(fn)                                                                                                                                         \
    do {                                                                                                                                                       \
        size_t const err = (fn);                                                                                                                               \
        CHECK(!ZSTD_isError(err), "%s", ZSTD_getErrorName(err));                                                                                               \
    } while (0)

#define CHECK_SQLITE(expr,ctx, ...)                                                                                                                                \
    do {                                                                                                                                                       \
        int status = expr;                                                                                                                                     \
        if (!((status == SQLITE_OK) || (status == SQLITE_DONE) || (status == SQLITE_ROW))) {                                                                   \
            fprintf(stderr, "%s:%d CHECK(%s) failed: ", __FILE__, __LINE__, #expr);                                                                            \
            fprintf(stderr, "%s\n", sqlite3_errmsg(ctx->DB));                                                                                                    \
            fprintf(stderr, "" __VA_ARGS__);                                                                                                                   \
            fprintf(stderr, "\n");                                                                                                                             \
            exit(1);                                                                                                                                           \
        }                                                                                                                                                      \
    } while (0)

typedef struct {
    long nprocs;
    long nprocs_max;
} cpu_threads_t;

cpu_threads_t* get_cpu_threads();

#endif