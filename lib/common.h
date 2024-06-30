#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "container.h"

extern db_ctx db;

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

#define CHECK_SQLITE(stmt, ...)                                                                                                                                \
    do {                                                                                                                                                       \
        int status = stmt;                                                                                                                                     \
        if (!((status == SQLITE_OK) || (status == SQLITE_DONE) || (status == SQLITE_ROW))) {                                                                   \
            fprintf(stderr, "%s:%d CHECK(%s) failed: ", __FILE__, __LINE__, #stmt);                                                                            \
            fprintf(stderr, "%s\n", sqlite3_errmsg(db.DB));                                                                                                    \
            fprintf(stderr, "" __VA_ARGS__);                                                                                                                   \
            fprintf(stderr, "\n");                                                                                                                             \
            exit(1);                                                                                                                                           \
        }                                                                                                                                                      \
    } while (0)

#endif