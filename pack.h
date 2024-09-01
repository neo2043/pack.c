#ifndef PACK
#define PACK

#include "arraylist.h"

typedef struct {
    char *file_path;
    char *rel_file_path;
    long long int file_size;
    arraylist* hashtable;
} thread_compression_function_arg_t;

typedef struct {
    unsigned long long int id;
    char *file_path;
    arraylist* hashtable;
} thread_decompression_function_arg_t;

// void singlethread_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread);
void mt_compress_handler(char *f_path, char *db_path, int compression_level, int num_thread,int verbose);
void* mt_compression_function(void *arg);
void mt_decompress_handler(char *d_f_path, char *db_path, int num_thread,int verbose);
void *mt_decompression_function(void *arg);

#endif