#include "../lib/helpers.h"

int main(int argc, char **argv) {
    char buf[4096 + 1]; // +1 for delimiter
    int tot_read = 0;
    while ((tot_read = read_until(STDIN_FILENO, buf, 4096 + 1, '\n')) > 0) {
        buf[tot_read - 1] = '\0';
        char *args[argc + 1];
        int i;
        for (i = 1; i < argc; i++) {
            args[i - 1] = argv[i];
        }
        args[argc - 1] = buf;
        args[argc] = NULL;
        if (spawn(argv[1], args) == 0) {
            write_(STDOUT_FILENO, buf, tot_read);
        }
    }
    return 0;
}
