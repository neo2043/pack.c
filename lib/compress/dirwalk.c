#include <dirent.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "arraylist.h"
#include "cwalk.h"
#include "dirwalk.h"

char *walk_get_absolute_path(walk_ctx *pathCtx, int index, int only_relative_path) {
    char *path = calloc(4, sizeof(char));
    if (!only_relative_path) {
#if defined(_WIN32)
        size_t rootlen;
        cwk_path_get_root(((segment_t *)arraylist_get(pathCtx->path_segment_stack, 0))->segment_name, &rootlen);
        memcpy(path, ((segment_t *)arraylist_get(pathCtx->path_segment_stack, 0))->segment_name, rootlen);
#elif defined(__unix__) || defined(__linux__)
        strcpy(path, "/");
#endif
    }
    arraylist_iterate(pathCtx->path_segment_stack) {
        if (only_relative_path && arraylist_size(pathCtx->path_segment_stack) == 1 && S_ISREG(pathCtx->stbuf->st_mode)) {
            const char *base;
            size_t len;
            cwk_path_get_basename(((segment_t *)ctx.item)->segment_name, &base, &len);
            path = realloc(path, sizeof(char) * (len + 5));
            memcpy(path, base, len);
            return path;
        }
        if (only_relative_path) {
            only_relative_path = 0;
            continue;
        }
        if (index != -1 && ctx.index > index) {
            continue;
        }
        int pathlen = ((strlen(((segment_t *)ctx.item)->segment_name) + 1) + (strlen(path) + 1));
        path = realloc(path, sizeof(char) * pathlen);
        cwk_path_join(path, ((segment_t *)ctx.item)->segment_name, path, pathlen);
    }
    return path;
}

static void add_new_seg_in_arr(walk_ctx *ctx, char *segment_name, long associated_no) {
    segment_t *new_seg = calloc(1, sizeof(segment_t));
    new_seg->segment_name = calloc(strlen(segment_name) + 1, sizeof(char));
    strcpy(new_seg->segment_name, segment_name);
    new_seg->associated_no = associated_no;
    new_seg->passed_over = false;
    arraylist_add(ctx->path_segment_stack, new_seg);
    char *new_seg_path = walk_get_absolute_path(ctx, -1, 0);
#if defined(_WIN32)
    _stat64(new_seg_path, ctx->stbuf);
#elif defined(__unix__) || defined(__linux__)
    stat(new_seg_path, ctx->stbuf);
#endif
    if (S_ISDIR(ctx->stbuf->st_mode)) {
        new_seg->seg_type = folder;
    } else if (S_ISREG(ctx->stbuf->st_mode)) {
        new_seg->seg_type = file;
    }
    free(new_seg_path);
}

walk_ctx init_walk_ctx(char *root_path) {
#if defined(_WIN32)
    walk_ctx ctx = {.path_segment_stack = arraylist_create(), .stbuf = calloc(1, sizeof(struct _stat64))};
#elif defined(__unix__) || defined(__linux__)
    walk_ctx ctx = {.path_segment_stack = arraylist_create(), .stbuf = calloc(1, sizeof(struct stat))};
#endif
    int path_len = strlen(root_path) < PATH_MAX ? PATH_MAX : (strlen(root_path) + 1);
    segment_t *first_seg = calloc(1, sizeof(segment_t));
    first_seg->segment_name = calloc(path_len, sizeof(char));
    getcwd(first_seg->segment_name, path_len);
    cwk_path_get_absolute(first_seg->segment_name, root_path, first_seg->segment_name, path_len);
    first_seg->associated_no = 0;
    first_seg->passed_over = false;
#if defined(_WIN32)
    _stat64(first_seg->segment_name, ctx.stbuf);
#elif defined(__unix__) || defined(__linux__)
    stat(first_seg->segment_name, ctx.stbuf);
#endif
    if (S_ISDIR(ctx.stbuf->st_mode)) {
        first_seg->seg_type = folder;
    } else if (S_ISREG(ctx.stbuf->st_mode)) {
        first_seg->seg_type = file;
    }
    arraylist_add(ctx.path_segment_stack, first_seg);
    return ctx;
}

static void deinit_walk_segment(segment_t *temp) {
    free(temp->segment_name);
    free(temp);
}

void deinit_walk_ctx(walk_ctx *ctx) {
    free(ctx->stbuf);
    for (int i = 0; i < arraylist_size(ctx->path_segment_stack); i++) {
        segment_t *temp = arraylist_pop(ctx->path_segment_stack);
        deinit_walk_segment(temp);
    }
    arraylist_destroy(ctx->path_segment_stack);
}

static struct dirent *readdir_handler(DIR *dir) {
    struct dirent *de;
    do {
        de = readdir(dir);
        if (de == NULL) { return NULL; }
    } while (!(strcmp(de->d_name, ".")) || !(strcmp(de->d_name, "..")));
    return de;
}

int walk_next(walk_ctx *ctx) {
    segment_t *lst_seg = arraylist_get(ctx->path_segment_stack, arraylist_size(ctx->path_segment_stack) - 1);
    int freed = 0;
    if (lst_seg->seg_type == folder) {
        DIR *dir;
        if (lst_seg->passed_over) {
            if (lst_seg->associated_no == 0) { return 0; }
            char *passed_over_Path = walk_get_absolute_path(ctx, arraylist_size(ctx->path_segment_stack) - 2, 0);
            dir = opendir(passed_over_Path);
            free(passed_over_Path);
            seekdir(dir, lst_seg->associated_no);
            segment_t *temp = arraylist_pop(ctx->path_segment_stack);
            deinit_walk_segment(temp);
            freed = 1;
        } else {
            char *abs_path = walk_get_absolute_path(ctx, -1, 0);
            dir = opendir(abs_path);
            free(abs_path);
        }
        struct dirent *de;
        if(!freed){
            lst_seg->passed_over = true;
        }
        if ((de = readdir_handler(dir)) == NULL) {
            closedir(dir);
            return walk_next(ctx);
        }
        add_new_seg_in_arr(ctx, de->d_name, telldir(dir));
        closedir(dir);
    } else {
        if (lst_seg->associated_no == 0 && lst_seg->passed_over == false) {
            lst_seg->passed_over = true;
            return 1;
        }
        if (lst_seg->associated_no == 0 && lst_seg->passed_over == true) { return 0; }
        DIR *dir;
        struct dirent *de;
        char *abs_path = walk_get_absolute_path(ctx, arraylist_size(ctx->path_segment_stack) - 2, 0);
        dir = opendir(abs_path);
        free(abs_path);
        seekdir(dir, lst_seg->associated_no);
        if ((de = readdir_handler(dir)) == NULL) {
            segment_t *temp = arraylist_pop(ctx->path_segment_stack);
            deinit_walk_segment(temp);
            closedir(dir);
            return walk_next(ctx);
        }
        segment_t *temp = arraylist_pop(ctx->path_segment_stack);
        deinit_walk_segment(temp);
        add_new_seg_in_arr(ctx, de->d_name, telldir(dir));
        closedir(dir);
    }
    return 1;
}