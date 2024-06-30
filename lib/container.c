#include <stdio.h>
#include <string.h>

#include "common.h"
#include "sqlite3.h"
#include "container.h"

extern db_ctx db;

char *sqlcreatefiletable = "CREATE TABLE IF NOT EXISTS FILE_TABLE("
                           "ID          INTEGER    NOT NULL  PRIMARY KEY  AUTOINCREMENT, "
                           "FILE_PATH   TEXT       MOT NULL  UNIQUE);";

char *sqlcreatechunktable = "CREATE TABLE IF NOT EXISTS CHUNKS("
                            "ID       INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                            "SIZE     INTEGER NOT NULL, "
                            "FILE_ID  INTEGER NOT NULL, "
                            "OFFSET   INTEGER NOT NULL, "
                            "CHUNK    BLOB    NOT NULL, "

                            "FOREIGN KEY(FILE_ID) REFERENCES FILE_TABLE(ID) ON DELETE CASCADE);";

char *sqlcreatechunkindex = "CREATE UNIQUE INDEX IF NOT EXISTS CHUNKS_INDEX "
                            "ON CHUNKS (FILE_ID,OFFSET);";

char *insertinfiletable = "INSERT INTO FILE_TABLE (FILE_PATH) VALUES (?) RETURNING ID;";

char *insertinchunktable = "INSERT INTO CHUNKS (SIZE,FILE_ID,OFFSET,CHUNK) VALUES (?,?,?,?)";

char *listfiletable = "SELECT ID,FILE_PATH FROM FILE_TABLE;";

char *getfilesize = "SELECT SUM(SIZE) FROM chunks WHERE file_id = ?;";

char *getfilechunks = "SELECT ID, SIZE, OFFSET FROM CHUNKS WHERE FILE_ID = ? "
                      "ORDER BY OFFSET ASC;";

void create_table(void) {
    char *messageError;
    CHECK_SQLITE(sqlite3_exec(db.DB, sqlcreatefiletable, NULL, 0, &messageError), "create file table");
    CHECK_SQLITE(sqlite3_exec(db.DB, sqlcreatechunktable, NULL, 0, &messageError), "create chunk table");
    CHECK_SQLITE(sqlite3_exec(db.DB, sqlcreatechunkindex, NULL, 0, &messageError), "create chunk index");
}

void sqlstmtprepare() {
    CHECK_SQLITE(sqlite3_prepare_v2(db.DB, insertinfiletable, strlen(insertinfiletable), &db.insertinfiletablestmt, NULL), "file table stmt prepare");
    CHECK_SQLITE(sqlite3_prepare_v2(db.DB, insertinchunktable, strlen(insertinchunktable), &db.insertinchunkstablestmt, NULL), "chunk table stmt prepare");
}

int callback(void *ctx, int argc, char **argv, char **columns) {
    for (int i = 0; i < argc; i++) {
        printf("%s : %s ", columns[i], argv[i] ? argv[i] : "NULL");
    }
    printf("\n");
    return 0;
}