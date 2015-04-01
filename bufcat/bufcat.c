#include "../lib/bufio.h"
#include <limits.h>

const size_t MAX_SIZE = 1024*1024*1024; // 1 GB

int main() {
    struct buf_t *file = buf_new(MAX_SIZE);
    buf_fill(STDIN_FILENO, file, MAX_SIZE);
    buf_flush(STDOUT_FILENO, file, MAX_SIZE);
    return 0;
}
