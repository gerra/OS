#include "../lib/helpers.h"
#include "../lib/bufio.h"
#include <stdio.h>

const int MAX_LEN = 4096;
const int MAX_PIPE_ARGS = 4096;
const int MAX_PER_PIPE_ARGS = 4096;
const int MAX_PROGRAMS = 4096;

int split(char *s, char *buf[]) {
    int len = strlen(s);

    int argc = 0;
    char curArg[len];
    int offset = 0;
    int i = 0;
    while (s[i] == ' ') i++;
    for (; i <= len; i++) {
        if (s[i] == ' ' || i == len) {
            if (s[i] == ' ') {
                while (s[i] == ' ') i++;
                i--;
            }
            curArg[offset] = 0;
            buf[argc] = malloc(sizeof(char) * offset);
            memcpy(buf[argc], curArg, offset * sizeof(char));
            argc++;
            offset = 0;
        } else {
            curArg[offset++] = s[i];
        }
    }
    return argc;
}

void sig_handler(int signo) {
    if (signo == SIGINT) {
        write_(STDOUT_FILENO, "\n$ ", 3);
    }
}

int main(int argc, int **argv) {
    while (1) {
        write_(STDOUT_FILENO, "$ ", 2);
        signal(SIGINT, sig_handler);
        char *pipeArgs[MAX_PIPE_ARGS];
        int pipeArgCnt = 0;

        char curArg[MAX_LEN];
        int offset = 0;

        int len;
        char buf[MAX_LEN];
        struct buf_t *bbuf = buf_new(4096);

        while (1) {
            len = buf_getline(STDIN_FILENO, bbuf, buf);
            if (len <= 0) break;
            if (len != MAX_LEN) {
                buf[len] = 0;
            }
            int i;
            int endOfComand = 0;
            for (i = 0; i < len; i++) {
                if (buf[i] == '|' || buf[i] == '\n') {
                    while (offset > 0 && curArg[offset-1] == ' ') {
                        offset--;
                        curArg[offset] = 0;
                    }
                    pipeArgs[pipeArgCnt] = malloc(sizeof(char) * offset);
                    memcpy(pipeArgs[pipeArgCnt], curArg, offset * sizeof(char));
                    if (offset != 0) {
                        pipeArgCnt++;
                    }
                    offset = 0;

                    if (buf[i] == '\n') {
                        endOfComand = 1;
                        break;
                    }
                } else {
                    if (offset != 0 || buf[i] != ' ') {
                        curArg[offset++] = buf[i];
                    }
                }
            }
            if (endOfComand) {
                break;
            }
        }
        //printf("%d\n%s\n%d\n", len, buf, bbuf->fill_size);
        if (len == 0 && offset == 0 && pipeArgCnt == 0) {
            write(STDOUT_FILENO, "\n", 1);
            break;
        }
        struct execargs_t *programs[MAX_PROGRAMS];
        int i;
        for (i = 0; i < pipeArgCnt; i++) {
            char *argv0[MAX_PER_PIPE_ARGS];
            int argc = split(pipeArgs[i], argv0);
            programs[i] = build_execargs(argc, argv0);
        }

        int res = runpiped(programs, pipeArgCnt);
    }
   return 0;
}
