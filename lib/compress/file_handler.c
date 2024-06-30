#include <ftw.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "file_handler.h"
#include "sqlite3.h"
#include "container.h"
#include "zstd_util.h"

extern db_ctx db;

int file_handler(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf) {
    if (tflag == FTW_F) {
        CHECK_SQLITE(sqlite3_bind_text(db.insertinfiletablestmt, 1, fpath, strlen(fpath), NULL), "insert file table text bind");
        CHECK_SQLITE(sqlite3_step(db.insertinfiletablestmt), "insert file table step");
        int fileid = sqlite3_column_int(db.insertinfiletablestmt, 0);

        compress_ctx rsc = init_compress_ctx();
        FILE *const f = fopen(fpath, "rb");
        for (int bytesLeft = sb->st_size; bytesLeft > 0;) {
            int readByteCount = fread(rsc.fBuffer, sizeof(char), rsc.fBufferSize, f);
            int cSize = ZSTD_compressCCtx(rsc.cctx, rsc.cBuffer, rsc.cBufferSize, rsc.fBuffer, readByteCount, 9);
            CHECK_ZSTD(cSize);
            CHECK_SQLITE(sqlite3_bind_int(db.insertinchunkstablestmt, 1, readByteCount), "insert chunk table int (size) bind");
            CHECK_SQLITE(sqlite3_bind_int(db.insertinchunkstablestmt, 2, fileid), "insert chunk table int (file_id) bind");
            CHECK_SQLITE(sqlite3_bind_int(db.insertinchunkstablestmt, 3, sb->st_size - bytesLeft), "insert chunk table int (offset) bind");
            CHECK_SQLITE(sqlite3_bind_blob(db.insertinchunkstablestmt, 4, rsc.cBuffer, cSize, NULL), "insert chunk table chunk bind");
            CHECK_SQLITE(sqlite3_step(db.insertinchunkstablestmt), "insert chunk table chunk step");
            CHECK_SQLITE(sqlite3_reset(db.insertinchunkstablestmt), "insert in chunk table stmt reset");
            bytesLeft -= readByteCount;
        }
        fclose(f);
        CHECK_SQLITE(sqlite3_reset(db.insertinfiletablestmt), "insert in file table stmt reset");
    }
    return FTW_CONTINUE; /* To tell nftw() to continue */
}