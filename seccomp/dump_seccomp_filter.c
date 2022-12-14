/*************************************************************************\
*                  Copyright (C) Michael Kerrisk, 2022.                   *
*                                                                         *
* This program is free software. You may use, modify, and redistribute it *
* under the terms of the GNU General Public License as published by the   *
* Free Software Foundation, either version 3 or (at your option) any      *
* later version. This program is distributed without any warranty.  See   *
* the file COPYING.gpl-v3 for details.                                    *
\*************************************************************************/

/* Supplementary program for Chapter Z */

#include <linux/seccomp.h>
#include <linux/filter.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)

/* Fetch BPF filter with specified index for specified PID. A pointer to a
   dynamically allocated buffer is containing the filter code returned as the
   function result; the caller is responsible for freeing this buffer. If
   'instrCnt' is not NULL, then the number of instructions in the filter is
   placed in *instrCnt. */

static struct sock_filter *
fetchFilter(pid_t pid, int filterIndex, int *instrCnt)
{
    /* Attach to the target process and wait for it to be stopped by
       the attach operation */

    if (ptrace(PTRACE_ATTACH, pid, 0, 0) == -1)
        errExit("ptrace - PTRACE_ATTACH");

    if (waitpid(pid, NULL, 0) == -1)
        errExit("waitpid");

    /* Discover the number of instructions in the BPF filter */

    int icnt = ptrace(PTRACE_SECCOMP_GET_FILTER, pid, filterIndex, NULL);
    if (icnt == -1) {
        if (errno == ENOENT) {
            fprintf(stderr, "No BPF program exists at index %d\n", filterIndex);
            exit(EXIT_FAILURE);
        } else if (errno == EACCES) {   /* As documented in ptrace(2)... */
            fprintf(stderr, "You lack the CAP_SYS_ADMIN capability; "
                    "run this program as root\n");
            exit(EXIT_FAILURE);
        } else {
            errExit("ptrace - PTRACE_SECCOMP_GET_FILTER-1");
        }
    }

    /* Allocate a buffer and fetch the content of the BPF filter */

    struct sock_filter *filterProg;
    filterProg = calloc(icnt, sizeof(struct sock_filter));
    if (filterProg == NULL)
        errExit("calloc");

    icnt = ptrace(PTRACE_SECCOMP_GET_FILTER, pid, filterIndex, filterProg);
    if (icnt == -1)
        errExit("ptrace - PTRACE_SECCOMP_GET_FILTER-2");

    if (instrCnt != NULL)
        *instrCnt = icnt;

    return filterProg;
}

/* Dump filter contents to a file */

static void
dumpFilter(char *pathname, struct sock_filter *filterProg, int instrCnt)
{
    int fd = open(pathname, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR);
    if (fd == -1)
        errExit("open");

    if (write(fd, filterProg, instrCnt * sizeof(struct sock_filter)) == -1)
        errExit("write");

    if (close(fd) == -1)
        errExit("close");

    fprintf(stderr, "Dumped %d BPF instructions\n", instrCnt);
}

int
main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "--help") == 0) {
        fprintf(stderr, "%s PID dump-file [filter-index]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    pid_t pid = atoi(argv[1]);
    int filterIndex = (argc > 3) ? atoi(argv[3]) : 0;

    int instrCnt;       /* Number of instructions in BPF filter */
    struct sock_filter *filterProg = fetchFilter(pid, filterIndex, &instrCnt);

    dumpFilter(argv[2], filterProg, instrCnt);

    free(filterProg);

    exit(EXIT_SUCCESS);
}
