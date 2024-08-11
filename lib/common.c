#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"

cpu_threads_t* get_cpu_threads(){
    cpu_threads_t *cpu_thread_info = malloc(sizeof(cpu_threads_t));
    cpu_thread_info->nprocs = -1;
    cpu_thread_info->nprocs_max = -1;
#ifdef _WIN32
#ifndef _SC_NPROCESSORS_ONLN
    SYSTEM_INFO info;
    GetSystemInfo(&info);
#define sysconf(a) info.dwNumberOfProcessors
#define _SC_NPROCESSORS_ONLN
#endif
#endif
#ifdef _SC_NPROCESSORS_ONLN
    cpu_thread_info->nprocs = sysconf(_SC_NPROCESSORS_ONLN);
    if (cpu_thread_info->nprocs < 1) {
        fprintf(stderr, "Could not determine number of CPUs online:\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    cpu_thread_info->nprocs_max = sysconf(_SC_NPROCESSORS_CONF);
    if (cpu_thread_info->nprocs_max < 1) {
        fprintf(stderr, "Could not determine number of CPUs configured:\n%s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    printf("%ld of %ld processors online\n", cpu_thread_info->nprocs, cpu_thread_info->nprocs_max);
    return cpu_thread_info;
    // exit(EXIT_SUCCESS);
#else
    fprintf(stderr, "Could not determine number of CPUs");
    exit(EXIT_FAILURE);
#endif
}