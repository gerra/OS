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
    char buf[2048];
    char for_write[1024];
    int for_write_it = 0;
    int it = 0;
    while ((tot_read = read_(STDIN_FILENO, buf, 1024)) > 0) {
        int k;

        int was_extra_read = 0;

        do {
            for (k = 0; k < tot_read;) {

                int j = 0;

                for (; delword[j] == buf[k + j] && j < delword_length; j++) {
                    if (k + j == tot_read - 1 && was_extra_read == 0) {
                        was_extra_read = read_(STDIN_FILENO, buf + tot_read, 1024);
                    }
                }

                if (delword_length != j) {
                    for_write_it = put_to(buf[k], for_write, for_write_it);
                    k++;
                } else {
                    k += j;
                }
            }

            int j;
            for (j = 0; j < was_extra_read; j++) {
                buf[j] = buf[tot_read + j];
            }
            tot_read = was_extra_read;
        } while (was_extra_read > 0);
    }
    write_(STDOUT_FILENO, for_write, for_write_it);
    exit(0);
}
