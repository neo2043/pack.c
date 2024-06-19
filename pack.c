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

int main() {
    arraylist* pathList = arraylist_create();
    unsigned int index;
    char *item;
    walk(".",pathList);
    arraylist_iterate(pathList){
        printf("[%i] = %p = %s\n", ctx.index, ctx.item, (char*)ctx.item);
    }
    return 0;
}