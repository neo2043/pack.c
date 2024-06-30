#ifndef FILE_HANDLER
#define FILE_HANDLER

#include <ftw.h>

int file_handler(const char *fpath, const struct stat *sb, int tflag, struct FTW *ftwbuf);

#endif // FILE_HANDLER