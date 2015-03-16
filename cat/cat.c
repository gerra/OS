#include "../lib/helpers.h"

int main() {
    char buf[1024];
    int tot_read = 0;
    while ((tot_read = read_(STDIN_FILENO, buf, 1024)) > 0) {
        write_(STDOUT_FILENO, buf, tot_read);
    }
    exit(0);
}
