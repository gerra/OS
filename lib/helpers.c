#include "helpers.h"
#include <stdio.h>

ssize_t read_(int fd, void *buf, size_t count) {
    int total_read = 0;
    int now_read;
    while (total_read < count && (now_read = read(fd, buf + total_read, count - total_read)) > 0) {
        total_read += now_read;
    }
    return total_read;
}

ssize_t write_(int fd, const void *buf, size_t count) {
    int total_written = 0;
    int now_written;
    while (total_written < count && (now_written = write(fd, buf + total_written, count - total_written)) > 0) {
        total_written += now_written;
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
    return total_read;
}

int spawn(const char * file, char * const argv []) {
    pid_t pid;
    int res;
    switch(pid = fork()) {
    case -1:
        exit(1);
    case 0:
        res = execvp(file, argv);
        exit(res);
    default:
        wait();
        return res;
    }
}

int str_len(char *s) {
    int len = 0;
    for (; s[len++] != '\0';);
    len--;
    return len;
}
