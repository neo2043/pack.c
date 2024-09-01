#ifndef COMMON_H
#define COMMON_H

#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

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

#define CHECK_SQLITE(expr, ctx, ...)                                                                                                                           \
    do {                                                                                                                                                       \
        int status = expr;                                                                                                                                     \
        if (!((status == SQLITE_OK) || (status == SQLITE_DONE) || (status == SQLITE_ROW))) {                                                                   \
            fprintf(stderr, "thread_no: %lld\n", pthread_self());                                                                                              \
            fprintf(stderr, "%s:%d CHECK(%s) failed with code %d: ", __FILE__, __LINE__, #expr, status);                                                       \
            fprintf(stderr, "%s\n", sqlite3_errmsg(ctx->DB));                                                                                                  \
            fprintf(stderr, "" __VA_ARGS__);                                                                                                                   \
            fprintf(stderr, "\n");                                                                                                                             \
            exit(1);                                                                                                                                           \
        }                                                                                                                                                      \
    } while (0)

#define CHECK_SQLITE_MULTITHREAD(expr, ctx, ...)                                                                                                               \
    do {                                                                                                                                                       \
        int continueTrying = 1;                                                                                                                                \
        while (continueTrying) {                                                                                                                               \
            int status = expr;                                                                                                                                 \
            if (status == SQLITE_BUSY) {                                                                                                                       \
                sleep(1);                                                                                                                                      \
                continue;                                                                                                                                      \
            }                                                                                                                                                  \
            if (!((status == SQLITE_OK) || (status == SQLITE_DONE) || (status == SQLITE_ROW))) {                                                               \
                fprintf(stderr, "%d\n", status);                                                                                                               \
                fprintf(stderr, "%s:%d CHECK(%s) failed: ", __FILE__, __LINE__, #expr);                                                                        \
                fprintf(stderr, "%s\n", sqlite3_errmsg(ctx->DB));                                                                                              \
                fprintf(stderr, "" __VA_ARGS__);                                                                                                               \
                fprintf(stderr, "\n");                                                                                                                         \
                exit(1);                                                                                                                                       \
            }                                                                                                                                                  \
            continueTrying = 0;                                                                                                                                \
        }                                                                                                                                                      \
    } while (0)

typedef struct {
    long nprocs;
    long nprocs_max;
} cpu_threads_t;

cpu_threads_t *get_cpu_threads();
int contains(const char *look_into, const char *look_for);
void change_path_type_to_unix(char *path);
void strncpy_m(char *dest, char *src, int len);
int make_nested_folder(char *root_path, char *nested_path);

#endif