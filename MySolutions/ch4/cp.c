#include <fcntl.h>
#include "tlpi_hdr.h"

#define BUF_SIZE 1024

int main(int argc, char *argv[]) {
    if(argc <= 2)
        usageErr("usage: %s srcfile desfile\n", argv[0]);
    const char *srcfile;
    const char *desfile;
    int sfd, dfd, readNum, ret;
    char* buf[BUF_SIZE];

    srcfile = argv[1];
    desfile = argv[2];
    sfd = open(srcfile, O_RDONLY);
    if(sfd == -1)
        errExit("open");
    dfd = open(desfile, O_WRONLY | O_TRUNC | O_CREAT);
    if(dfd == -1)
        errExit("open");

    while((readNum = read(sfd, buf, BUF_SIZE)) > 0) {
        ret = write(dfd, buf, readNum);
        if(ret == -1)
            errExit("write");
    }
    if(readNum == -1)
        errExit("read");
    
    if(close(sfd))
        errExit("close");
    if(close(dfd))
        errExit("close");

    return 0;
}