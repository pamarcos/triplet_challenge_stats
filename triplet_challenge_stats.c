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

#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/stat.h> /* For mode constants */
#include <fcntl.h> /* For O_* constants */ 
#include <stdlib.h>

#include "triplet_challenge_stats.h"

static int shared_mem_fd = 0;

void openSharedMem()
{
    printf("Opening shared memory %s...\n", TCS_SHARED_MEM_NAME);
    shared_mem_fd = shm_open(TCS_SHARED_MEM_NAME, O_CREAT | O_RDONLY, 0666);
    if (shared_mem_fd < 0)
        printf("Error opening shared memory %s: %s (%d)\n", TCS_SHARED_MEM_NAME, strerror(errno), errno);
}

void closeSharedMem()
{
    printf("Closing shared memory %s...\n", TCS_SHARED_MEM_NAME);
    int err = shm_unlink(TCS_SHARED_MEM_NAME);
    if (err != 0)
        printf("Error unlinking shared memory area %s\n", TCS_SHARED_MEM_NAME);
}

void readSharedMem()
{
    openSharedMem();

    FILE *file = fdopen(shared_mem_fd, "r");
    if (!file)
        printf("Error opening file %s for reading: %s (%d)", TCS_SHARED_MEM_NAME, strerror(errno), errno);

    char *line = NULL;
    ssize_t read = 0;
    size_t len = 0;
    size_t lines = 0;
    while ((read = getline(&line, &len, file)) != -1)
        lines++;

    double numbers[lines];
    double average;
    rewind(file);
    lines = 0;
    while ((read = getline(&line, &len, file)) != -1) {
        printf("%s", line);
        numbers[lines] = strtold(line, NULL);
        average += numbers[lines];
        lines++;
    }
    average /= lines;
    printf("Average time for the %zu samples: %.2f ms\n", lines, average);

    free(line);

    fclose(file);
}

void help()
{
    printf("Options:\n"
        "open  - opens the shared memory area\n"
        "close - closes the shared memory area\n"
        "clean - cleans the shared memory area\n"
        "read  - reads the values in shared memory area\n"
        "help  - shows this help\n");
}

int main(int argc, char *argv[])
{
    printf("triplet_challenge_stats\n");
    if (argc > 1) {
        if (strcmp(argv[1], "open") == 0) {
            openSharedMem();
        } else if (strcmp(argv[1], "close") == 0) {
            closeSharedMem();
        } else if (strcmp(argv[1], "clean") == 0) {
            closeSharedMem();
            openSharedMem();
        } else if (strstr(argv[1], "help")) {
            help();
        } else {
            readSharedMem();
        }
    } else {
        readSharedMem();
    }
    return 0;
}
