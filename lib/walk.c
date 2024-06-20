#include <assert.h>
#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "arraylist.h"
#include "cwalk.h"
#include "walk.h"

void walk(char *path, arraylist *d_arr) {
    DIR *dirhandle = opendir(path);
    assert(dirhandle);
    struct dirent *de;
    while ((de = readdir(dirhandle)) != NULL) {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }
        _Bool is_dir;
        _Bool is_reg;
        struct stat stbuf;
        int newPathLen = strlen(path) + strlen(de->d_name) + 5;
        char *newPathBuffer = calloc(newPathLen, sizeof(char));
        cwk_path_join(path, de->d_name, newPathBuffer, newPathLen);
        stat(newPathBuffer, &stbuf);
        is_dir = S_ISDIR(stbuf.st_mode);
        is_reg = S_ISREG(stbuf.st_mode);
        if (is_dir) {
            walk(newPathBuffer, d_arr);
            free(newPathBuffer);
        } else if (is_reg) {
            struct stat *stbuf_copy = calloc(1, sizeof(stat));
            file_t *fileStats = calloc(1, sizeof(file_t));
            memmove(stbuf_copy, &stbuf, sizeof(stat));
            fileStats->filestat = stbuf_copy;
            fileStats->path = newPathBuffer;
            arraylist_add(d_arr, fileStats);
        }
    }
}
