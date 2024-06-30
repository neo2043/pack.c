#include <assert.h>
#include <dirent.h>
#include <ftw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "compress/file_handler.h"
#include "sqlite3.h"
#include "container.h"
#include "zstd.h"

db_ctx db;
void compress(void);

int main() {
    char *db_path = "D:\\all\\prog_elec_phil\\projects\\pack\\db\\p.db";
    // char *db_path = ":memory:";

    if (SQLITE_OK != sqlite3_open(db_path, &db.DB)) {
        printf("error opening sqlite db");
    }

    compress();

    exit(EXIT_SUCCESS);
}

void compress(void){
    int flags = 0;
    flags |= FTW_ACTIONRETVAL;
    create_table();

    sqlstmtprepare();

    if (nftw(".", file_handler, 20, flags) == -1) {
        perror("nftw");
        exit(EXIT_FAILURE);
    }

    sqlite3_finalize(db.insertinfiletablestmt);
    printf("finalize %s\n", sqlite3_errmsg(db.DB));
}