#ifndef PACK
#define PACK

#include "arraylist.h"

typedef struct {
    char *file_path;
    char *rel_file_path;
    long long int file_size;
    arraylist* hashtable;
} thread_function_arg_t;


void singlethread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread);
void multithread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread);
void* mt_compression_function(void *arg);

#endif