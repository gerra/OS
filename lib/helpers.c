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
    pid_t child_pid;
    int res;
    switch(child_pid = fork()) {
    case -1:
        exit(1);
    case 0:
        res = execvp(file, argv);
        exit(res);
    default:
       wait(&res);
    }
    return res;
}

int str_len(char *s) {
    int len = 0;
    for (; s[len++] != '\0';);
    len--;
    return len;
}

struct execargs_t *build_execargs(int argc, char **argv) {
    struct execargs_t *args = malloc(sizeof(struct execargs_t));
    args->argc = argc;
    args->argv = malloc(sizeof(char*) * (argc+3));
    int i;
    for (i = 0; i < argc; i++) {
        args->argv[i] = argv[i];
    }
    return args;
}

int exec(struct execargs_t *args) {
    return spawn(args->argv[0], args->argv);
}

void close_all_fd(int *fd, int cnt) {
    int i;
    for (i = 0; i < cnt; i++) {
        close(fd[i]);
    }
}

int runpiped(struct execargs_t** programs, size_t n) {
    int pipes[(n - 1) * 2];
    int i;
    for (i = 0; i < n - 1; i++) {
        pipe(pipes + i * 2);
    }
    pid_t pid = fork();
    if (pid < 0) {
        close_all_fd(pipes, (n - 1) * 2);
        exit(2);
    }
    if (pid == 0) {
        /* Child process */
        if (n != 1) {
            dup2(pipes[1], STDOUT_FILENO);
        }
        close_all_fd(pipes, (n - 1) * 2);
        exec(programs[0]);
    } else {
        for (i = 0; i < n-1; i++) {
            pid = fork();
            if (pid < 0) {
                close_all_fd(pipes, (n - 1) * 2);
                exit(2);
            }
            if (pid == 0) {
                /* New child process */
                dup2(pipes[i * 2], STDIN_FILENO);
                if (i + 1 != n-1) {
                    dup2(pipes[i * 2 + 3], STDOUT_FILENO);
                }

                close_all_fd(pipes, (n - 1) * 2);
                exec(programs[i + 1]);
            }
        }
    }
    close_all_fd(pipes, (n - 1) * 2);
    int res;
    for (i = 0; i < n; i++) {
        wait(&res);
    }
    return res;
}
