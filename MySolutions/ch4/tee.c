#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 1024

extern int optind, opterr, optopt;
extern char* optarg;

int main(int argc, char* argv[]) {
    if(argc < 2 || strcmp(argv[1], "--help") == 0)
        usageErr("usage: %s {-a} filename\n", argv[0]);
    int readNum, writeNum, fd, opt, flag;
    char buf[BUF_SIZE];
    const char* filename;

    /* 解析命令行，只处理一个文件 */
    opt = getopt(argc, argv, "a:");
    if(opt == -1 && optind < argc && argc == 2) {
        filename = argv[optind];
        //printf("%s\n", filename);
        flag = O_TRUNC;
    }else if(opt == 'a' && argc == 3) {
        filename = optarg;
        //printf("%s\n", filename);
        flag = O_APPEND;
    }else {
        cmdLineErr("usage: %s {-a} filename", argv[0]);
    }

    fd = open(filename, O_WRONLY | O_CREAT | flag, 0666);
    if(fd == -1)
        errExit("open");
    while ((readNum = read(STDIN_FILENO, buf, BUF_SIZE)) > 0) {
        writeNum = write(fd, buf, readNum);
        if(writeNum == -1)
            errExit("write:");
        writeNum = write(STDOUT_FILENO, buf, readNum);
        if(writeNum == -1)
            errExit("write:");
    }
    if(readNum == -1)
        errExit("read");
    if(close(fd) == -1)
        errExit("close");

    return 0;
}