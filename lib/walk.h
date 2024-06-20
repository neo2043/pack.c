#ifndef WALK
#define WALK

#include "arraylist.h"
void walk(char *path, arraylist *d_arr);

typedef struct{
    char *path;
    struct stat *filestat;
} file_t;

#endif  // WALK