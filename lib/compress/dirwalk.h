#ifndef WALK
#define WALK

#include "arraylist.h"

typedef enum { folder, file } path_segment_type;

typedef struct {
    char *segment_name;
    long associated_no;
    path_segment_type seg_type;
    int passed_over;
} segment_t;

typedef struct {
    arraylist *path_segment_stack;
#if defined(_WIN32)
    struct _stat64 *stbuf;
#elif defined(__unix__) || defined(__linux__)
    struct stat *stbuf;
#endif
    // struct stat *stbuf;
} walk_ctx;

walk_ctx init_walk_ctx(char *root_path);
void deinit_walk_ctx(walk_ctx *ctx);
int walk_next(walk_ctx *ctx);
char *walk_get_absolute_path(walk_ctx *pathCtx, int index, int only_relative_path);

#endif  // WALK