#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>

#include "walk.h"
#include "arraylist.h"
#include "sqlite3.h"

int main() {
    arraylist* pathList = arraylist_create();
    unsigned int index;
    char *item;
    walk(".",pathList);
    arraylist_iterate(pathList){
        printf("[%i] = stat* [%p] = %p = %s\n", ctx.index, ((file_t*)ctx.item)->filestat, ctx.item, (char*)((file_t*)ctx.item)->path);
    }
    return 0;
}