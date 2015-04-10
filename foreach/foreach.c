#include "../lib/bufio.h"
#include "../lib/helpers.h"
#include <stdio.h>

const size_t MAX_SIZE = 1024*1024*1024; // 1 GB

int main(int argc, char **argv) {
    struct buf_t *file = buf_new(MAX_SIZE);
   // char strings[100][4097];
    char buf[4097];

    int it = 0;
    int str_size = 0;

    while ((str_size = buf_getline(STDIN_FILENO, file, buf)) > 0) {
        if (!(str_size & 1)) {
            char *args[argc + 1];
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
            it++;
        }
        // for erase
        int i;
        for (i = 0; i <= str_size; i++) {
            buf[i] = 0;
        }
    }

    return 0;
}
