/**
 * triplet_challenge_stats is an application that collects stats for triplet_challenge

 * Copyright (C) 2019 Pablo Marcos Oltra
 *
 * This file is part of triplet_challenge_stats.
 *
 * triplet_challenge_stats is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * triplet_challenge_stats is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with triplet_challenge_stats.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _GNU_SOURCE

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */ 
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include "triplet_challenge_stats.h"

#define LOG(...) fprintf(stderr, "[triplet_challenge_stats] " __VA_ARGS__)

typedef int (*orig_open_ptr)(const char *pathname, int flags);
typedef int (*orig_close_ptr)(int fd);

static orig_open_ptr orig_open = NULL;
static orig_close_ptr orig_close = NULL;

static int shared_mem_fd = 0;

static const char *TCS_FILE_NAME = "pg2009.txt";
static int tcs_fd = 0;

static struct timespec start_time;

__attribute__((constructor)) void loaded()
{
    LOG("shared lib loaded\n");

    orig_open = (orig_open_ptr)dlsym(RTLD_NEXT, "open");
    if (!orig_open)
        LOG("Error getting open symbol: 0x%p\n", dlerror());

    orig_close = (orig_close_ptr)dlsym(RTLD_NEXT, "close");
    if (!orig_close)
        LOG("Error getting close symbol: 0x%p\n", dlerror());

    shared_mem_fd = shm_open(TCS_SHARED_MEM_NAME, O_CREAT | O_RDWR | O_APPEND, 0666);
    if (shared_mem_fd < 0)
        LOG("Error opening shared memory %s: %s (%d)\n", TCS_SHARED_MEM_NAME, strerror(errno), errno);
}

void stopTimer()
{
    static char buffer[32] = {0};

    struct timespec end_time;
    clock_gettime(CLOCK_REALTIME, &end_time);
    double elapsed = (end_time.tv_sec - start_time.tv_sec) * 1e3 + (end_time.tv_nsec - start_time.tv_nsec) / 1e6;
    LOG("end clock %f ms, elapsed time %f ms\n", end_time.tv_sec * 1e3 + end_time.tv_nsec / 1e6, elapsed);
    int size = sprintf(buffer, "%f\n", elapsed);
    write(shared_mem_fd, buffer, size);
    tcs_fd = 0;
}

__attribute__((destructor)) void unloaded()
{
    if (tcs_fd)
        stopTimer();
    LOG("shared shared lib unloaded\n");
}

int open(const char* pathname, int flags, ...)
{
    if (!orig_open)
        return -1;

    if (strstr(pathname, TCS_FILE_NAME)) {
        if (tcs_fd) {
            LOG("Error starting timer while it was already started\n");
            return -1;
        }
        clock_gettime(CLOCK_REALTIME, &start_time);
        LOG("Opening file %s: start clock %f ms\n", TCS_FILE_NAME, start_time.tv_sec * 1e3 + start_time.tv_nsec / 1e6);
        tcs_fd = orig_open(pathname, flags);
        return tcs_fd;
    }

    return orig_open(pathname, flags);
}

int close(int fd)
{
    if (!orig_close)
        return -1;

    if (fd == tcs_fd) {
        if (!tcs_fd) {
            LOG("Error stoping timer when it was not started\n");
            return -1;
        }

        LOG("Closing file %s: ", TCS_FILE_NAME);
        stopTimer();
        return 0;
    } else {
        return orig_close(fd);
    }
}
