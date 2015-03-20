#include "../lib/helpers.h"

int put_to(char c, char *buf, int it) {
    if (it == 1024) {
        write_(STDOUT_FILENO, buf, 1024);
        it = 0;
    }
    buf[it++] = c;
    return it;
}

int main(int argc, char **argv) {
    char *delword = argv[1];
    int delword_length = str_len(delword);

    int tot_read = 0;
    char buf[1024];
    char for_write[1024];
    int for_write_it = 0;
    int it = 0;
    while ((tot_read = read_(STDIN_FILENO, buf, 1024)) > 0) {
        int k;
        for (k = 0; k < tot_read; k++) {
            if (buf[k] != delword[it] || delword_length == 0) {
                int i;
                if (delword_length != 0) {
                    // write_(STDOUT_FILENO, delword, it);
                    int z;
                    for (z = 0; z < it; z++) {
                        for_write_it = put_to(delword[z], for_write, for_write_it);
                    }
                }
                // write_(STDOUT_FILENO, buf + k, 1);
                for_write_it = put_to(buf[k], for_write, for_write_it);
                it = 0;

            } else {
                it++;
                if (it == delword_length) {
                    it = 0;
                }
            }
        }
    }
    write_(STDOUT_FILENO, for_write, for_write_it);
    exit(0);
}
