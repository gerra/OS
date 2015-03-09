#include <stdio.h>
#include <errno.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int total_read = read(fd, buf, count);
    if (total_read < 0) {
        perror(errno);
    }
    return total_read;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int total_writed = write(fd, buf, count);
    if (total_writed < 0) {
        perror(errno);
    }
    return total_writed;
}