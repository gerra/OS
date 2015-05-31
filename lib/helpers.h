#ifndef HELPERS_H
#define HELPERS_H

#include <unistd.h>
#include <sys/wait.h>

struct execargs_t {
    int argc;
    char **argv;
};

extern struct execargs_t *build_execargs(int argc, char **argv);
extern int exec(struct execargs_t* args);
extern int runpiped(struct execargs_t** programs, size_t n);
extern ssize_t read_(int fd, void *buf, size_t count);
extern ssize_t write_(int fd, const void *buf, size_t count);
extern ssize_t read_until(int fd, void * buf, size_t count, char delimiter);
extern int spawn(const char * file, char * const argv []);
extern int str_len(char *s);


#endif
