#include "../lib/bufio.h"
#include "../lib/helpers.h"
#include <stdio.h>

const size_t MAX_SIZE = 1024 * 1024; // 1 MB

int main(int argc, char **argv) {
    struct buf_t *file = buf_new(MAX_SIZE);
    if (!file) {
        return 1;
    }

    char buf[4097];

    int str_size = 0;
    char *args[argc + 1];

    while ((str_size = buf_getline(STDIN_FILENO, file, buf)) > 0) {
        if (str_size % 2 == 0) {
            int i;
            for (i = 1; i < argc; i++) {
                args[i - 1] = argv[i];
            }
            args[argc - 1] = buf;
            args[argc] = NULL;
            if (spawn(argv[1], args) == 0) {
                buf[str_size] = '\n';
                write_(STDOUT_FILENO, buf, str_size+1);
            }
        }
    }
    if (str_size < 0) {
        return 1;
    }
    return 0;
}
