#ifndef SQLITE_UTIL
#define SQLITE_UTIL

#include "sqlite3.h"

typedef struct {
    sqlite3 *DB;
    sqlite3_stmt *insertinfiletablestmt;
    sqlite3_stmt *insertinchunkstablestmt;
} db_ctx;

extern char *sqlcreatefiletable;

extern char *sqlcreatechunktable;

extern char *sqlcreatechunkindex;

extern char *insertinfiletable;

extern char *insertinchunktable;

extern char *listfiletable;

extern char *getfilesize;

extern char *getfilechunks;

void create_table(void);

void sqlstmtprepare();

#endif // SQLITE_UTIL
