#include "../lib/bufio.h"
#include <limits.h>

const size_t MAX_SIZE = 1024 * 1024; // 1 MB

int main() {
    struct buf_t *file = buf_new(MAX_SIZE);
    if (!file) {
        return 1;
    }
    while (buf_fill(STDIN_FILENO, file, MAX_SIZE) > 0) {
        buf_flush(STDOUT_FILENO, file, MAX_SIZE);
    }
    return 0;
}
