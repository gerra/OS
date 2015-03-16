#include <stdio.h>
#include <errno.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int total_read = 0;
    int now_read;
    while (total_read < count && (now_read = read(fd, buf + total_read, count - total_read)) > 0) {
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
    while (total_written < count && (now_written = write(fd, buf + total_written, count - total_written)) > 0) {
        total_written += now_written;
    }
    if (now_written < 0) {
        perror(errno);
    }
    return total_written;
}

ssize_t read_until(int fd, void * buf, size_t count, char delimiter) {
    int total_read = 0;
    int now_read;
    char *it = buf;
    while (total_read < count && (now_read = read(fd, buf + total_read, 1)) > 0) {
        total_read += now_read;
        if (*((char*)buf + total_read - 1) == delimiter) {
            break;
        }
    }
    if (now_read < 0) {
        perror(errno);
    }
    return total_read;
}
