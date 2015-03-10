#include <stdio.h>
#include <errno.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int total_read = 0;
    int now_read;
    while (total_read < count && (now_read = read(fd, buf, count - total_read)) > 0) {
        total_read += now_read;
    }
    if (now_read < 0) {
        perror(errno);
    }
    return total_read;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int total_written = 0;
    int now_written;
    while (total_written < count && (now_written = write(fd, buf, count - total_written)) > 0) {
        total_written += now_written;
    }
    if (now_written < 0) {
        perror(errno);
    }
    return total_written;
}