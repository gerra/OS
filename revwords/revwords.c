#include "../lib/helpers.h"

int main() {
    char buf[4096 + 1]; // +1 for delimiter
    int tot_read = 0;
    while ((tot_read = read_until(STDIN_FILENO, buf, 4096 + 1, ' ')) > 0) {
        int i;
        int read_without_delim = tot_read;
        if (buf[tot_read - 1] == ' ') {
            read_without_delim--;
        }
        for (i = 0; i < read_without_delim / 2; i++) {
            char c = buf[i];
            buf[i] = buf[read_without_delim - 1 - i];
            buf[read_without_delim - 1 - i] = c;
        }
        write_(STDOUT_FILENO, buf, tot_read);
    }
    exit(0);
}
