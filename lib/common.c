#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(_WIN32)
#include <direct.h>
#endif

#include "common.h"
#include "cwalk.h"

cpu_threads_t *get_cpu_threads() {
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
    return cpu_thread_info;
#else
    fprintf(stderr, "Could not determine number of CPUs");
    exit(EXIT_FAILURE);
#endif
}

int contains(const char *look_into, const char *look_for) {
    int i = 0;
    const int look_for_len = strlen(look_for);
    while (look_into[i] != '\0') {
        int in_count = 0;
        while (look_into[i] == look_for[in_count]) {
            if ((++in_count) == look_for_len) { return 1; }
            i++;
        }
        i++;
    }
    return 0;
}

void change_path_type_to_unix(char *path) {
    if (cwk_path_guess_style(path) == CWK_STYLE_WINDOWS) {
        for (int i = 0; path[i] != '\0'; i++) {
            if (path[i] == '\\') { path[i] = '/'; }
        }
    }
}

void strncpy_m(char *dest, char *src, int len) {
    int i;
    for (i = 0; i < len; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
}

int make_nested_folder(char *root_path, char *nested_path) {
    struct stat root_path_permission;
    struct cwk_segment segment;
    char *temp_full_path;
    char folder_name[PATH_MAX];
    int len;
    stat(root_path, &root_path_permission);
    DIR *dir = opendir(root_path);
    if (dir == NULL) { return 0; }
    if (strlen(nested_path) == 0) { return 1; }
    closedir(dir);
    len = strlen(nested_path) + strlen(root_path) + 10;
    temp_full_path = calloc(len, sizeof(char));
    if (cwk_path_get_first_segment(nested_path, &segment)) {
        strncpy_m(folder_name, segment.begin, segment.size);
        cwk_path_join(root_path, folder_name, temp_full_path, len);
        cwk_path_normalize(temp_full_path, temp_full_path, len);
#if defined(_WIN32)
        _mkdir(temp_full_path);
#elif defined(__unix__) || defined(__linux__)
        mkdir(temp_full_path, root_path_permission.st_mode);
#endif
    }
    while (cwk_path_get_next_segment(&segment)) {
        strncpy_m(folder_name, segment.begin, segment.size);
        cwk_path_join(temp_full_path, folder_name, temp_full_path, len);
        cwk_path_normalize(temp_full_path, temp_full_path, len);
#if defined(_WIN32)
        _mkdir(temp_full_path);
#elif defined(__unix__) || defined(__linux__)
        mkdir(temp_full_path, root_path_permission.st_mode);
#endif
    }
    return 1;
}