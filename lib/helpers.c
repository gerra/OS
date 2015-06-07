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
    signal(SIGINT, SIG_DFL);
    execvp(args->argv[0], args->argv);
    return -1;
}

struct sigaction prev;
int nn;
int *ppids;

void stop_all() {
    int i;
    for (i = 0; i < nn; i++) {
        kill(ppids[i], SIGTERM);
        waitpid(ppids[i], NULL, 0);
    }
}

void signal_handler_pipe(int signo) {
    stop_all();
}

#define CHECK(__x)      \
    if (__x == -1) {    \
        stop_all();     \
        return -1;      \
    }

int close_all_fd(int *fd, int cnt) {
    int i;
    for (i = 0; i < cnt; i++) {
        CHECK(close(fd[i]));
    }
}


int runpiped(struct execargs_t** programs, size_t n) {
    if (n < 0) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }

    struct sigaction sa;
    sa.sa_handler = signal_handler_pipe;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    CHECK(sigaction(SIGINT, &sa, &prev));

    int pipes[(n - 1) * 2];
    int pids[n];
    int i;
    for (i = 0; i < n - 1; i++) {
        CHECK(pipe(pipes + i * 2));
    }
    nn = 0;
    pid_t pid = fork();
    CHECK(pid);

    ppids = pids;
    ppids[nn++] = pid;
    int res;
    if (pid == 0) {
        /* Child process */
        if (n != 1) {
            CHECK(dup2(pipes[1], STDOUT_FILENO));
        }
        if (close_all_fd(pipes, (n - 1) * 2) == -1) return -1;
        res = exec(programs[0]);
        return -1;
    } else {
        for (i = 0; i < n-1; i++) {
            pid = fork();
            CHECK(pid);
            ppids[nn++] = pid;
            if (pid == 0) {
                /* New child process */
                CHECK(dup2(pipes[i * 2], STDIN_FILENO));
                if (i + 1 != n-1) {
                    CHECK(dup2(pipes[i * 2 + 3], STDOUT_FILENO));
                }
                if (close_all_fd(pipes, (n - 1) * 2) == -1) return -1;
                res = exec(programs[i + 1]);
                return -1;
            }
        }
    }
    if (close_all_fd(pipes, (n - 1) * 2) == -1) return -1;
    for (i = 0; i < n; i++) {
        wait(NULL);
    }
    while (waitpid(-1, NULL, WNOHANG) > 0); // kill zombies
    return 0;
}
