#include "../lib/bufio.h"

const int MAX_SIZE = 1 * 1024 * 1024; // 1MB

int main() {
    struct buf_t *file = buf_new(MAX_SIZE);
    buf_fill(STDIN_FILENO, file, MAX_SIZE);
    buf_flush(STDOUT_FILENO, file, MAX_SIZE);
    return 0;
}
