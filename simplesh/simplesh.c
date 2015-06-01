#include "../lib/helpers.h"
#include <stdio.h>

const int MAX_LEN = 4096;
const int MAX_PIPE_ARGS = 4096;
const int MAX_PER_PIPE_ARGS = 4096;
const int MAX_PROGRAMS = 4096;

void test() {

    char *args1[4];
    args1[0] = "ls";

    char *args2[4];
    args2[0] = "grep";
    args2[1] = ".c";

    struct execargs_t *z1 = build_execargs(1, args1);
    struct execargs_t *z2 = build_execargs(2, args2);

    struct execargs_t *p[3];

    p[0] = z1;
    p[1] = z2;

    runpiped(p, 2);
}

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
   // for (i = 0; i < argc; i++) {
    //    printf("%s\n", buf[i]);
    //}
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
        while ((len = read_until(STDIN_FILENO, buf, 5, '\n')) > 0) {
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
                    offset = 0;
                    pipeArgCnt++;

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



      //  printf("%s\n", programs[0]->argv[0]);
      //  printf("%s\n", programs[1]->argv[0]);

        int res = runpiped(programs, pipeArgCnt);

        //printf("hhh\n");

       // test();
        //tt[0] = buf;
        //struct execargs_t *argss = build_execargs(1, tt);
        //struct execargs_t *zz[5];
        //zz[0] = argss;
        //printf("%s", zz[0]->argv[0]);
        //runpiped(zz, 1);
    }
   return 0;

/*

    char *args1[4];
    args1[0] = "echo";
    args1[1] = "-e";
    args1[2] = "aa.1\nbb.2\ncc.3";

    char *args2[4];
    args2[0] = "grep";
    args2[1] = "\.2";

    char *args3[4];
    args3[0] = "head";

    char *args4[4];
    args4[0] = "ls";

    struct execargs_t *z1 = build_execargs(3, args1);
    struct execargs_t *z2 = build_execargs(2, args2);
    struct execargs_t *z3 = build_execargs(1, args3);
    struct execargs_t *z4 = build_execargs(1, args4);

    struct execargs_t *p[3];

    p[0] = z1;
    p[1] = z2;
    p[2] = z3;

    runpiped(p, 2);
    return 0;*/
}
