#include "bufio.h"
#include "helpers.h"
#include <stdlib.h>

struct buf_t *buf_new(size_t capacity) {
    struct buf_t *res = malloc(sizeof(struct buf_t));
    if (res != NULL) {
        res->capacity = capacity;
        res->fill_size = 0;
    }
    return res;
}

void buf_free(struct buf_t *buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif // DEBUG
    if (buf->fill_size != 0) {
        free(buf->data);
    }
    free(buf);
}

size_t buf_capacity(struct buf_t *buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif // DEBUG
    return buf->capacity;
}

size_t buf_size(struct buf_t *buf) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
    }
    #endif // DEBUG
    return buf->fill_size;
}

ssize_t buf_fill(int fd, struct buf_t *buf, size_t required) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
        return -1;
    }
    #endif // DEBUG
    size_t total_read = 0;
    size_t now_read;
    char *tmp = malloc(buf->capacity);
    int eof = 0;
    while (1) {
        now_read = read(fd, tmp + total_read + buf->fill_size, buf->capacity - buf->fill_size - total_read);
        if (now_read < 0) {
            free(tmp);
            return -1;
        }
        if (now_read == 0) {
            eof = 1;
            break;
        }
        total_read += now_read;
        if (total_read >= required) {
            break;
        }
        if (buf->capacity - buf->fill_size - total_read == 0) {
            break;
        }
    }
    // save old data:
    size_t i;
    for (i = 0; i < buf->fill_size; i++) {
        tmp[i] = buf->data[i];
    }
    // delete old data
    free(buf->data);
    // update data
    buf->fill_size += total_read;
    buf->data = malloc(buf->fill_size);
    for (i = 0; i < buf->fill_size; i++) {
        buf->data[i] = tmp[i];
    }
    // delete temporary data
    free(tmp);
    #ifdef DEBUG
    if (buf->fill_size == buf->capacity && total_read < required && eof == 0) {
        abort();
    }
    #endif
    return buf->fill_size;
}

ssize_t buf_flush(int fd, struct buf_t *buf, size_t required) {
    #ifdef DEBUG
    if (buf == NULL) {
        abort();
        return -1;
    }
    #endif // DEBUG
    size_t total_written = 0;
    size_t now_written;
    while (1) {
        size_t try_to_write = buf->capacity - total_written;
        if (try_to_write > buf->fill_size) {
            try_to_write = buf->fill_size;
        }
        now_written = write(fd, buf->data + total_written, try_to_write);
        if (now_written < 0) {
            return -1;
        }
        if (now_written == 0) {
            break;
        }
        total_written += now_written;
        if (total_written > required) {
            break;
        }
        if (try_to_write - total_written == 0) {
            break;
        }
    }
    buf->fill_size -= total_written;
    char *tmp = malloc(buf->fill_size);
    size_t i;
    for (i = 0; i < buf->fill_size; i++) {
        tmp[i] = buf->data[i + total_written];
    }
    free(buf->data);
    buf->data = malloc(buf->fill_size);
    for (i = 0; i < buf->fill_size; i++) {
        buf->data[i] = tmp[i];
    }
    free(tmp);
    return total_written;
}

void erase_first_n(struct buf_t *buf, size_t n) {
    if (n <= 0) {
        return;
    }
    if (n > buf->fill_size) {
        n = buf->fill_size;
    }
    char *tmp = malloc(buf->fill_size - n);
    size_t i;
    for (i = n; i < buf->fill_size; i++) {
        tmp[i-n] = buf->data[i];
    }
    free(buf->data);
    buf->fill_size -= n;
    buf->data = malloc(buf->fill_size+1);
    for (i = 0; i < buf->fill_size; i++) {
        buf->data[i] = tmp[i];
    }
    free(tmp);
}

ssize_t buf_getline(int fd, struct buf_t *buf, char *dest) {
    size_t i;
    for (i = 0; i < buf->fill_size; i++) {
        char cur = *(buf->data + i);
        if (cur == '\n') {
            dest = '\0';
            erase_first_n(buf, i+1);
            return i;
        }
        *dest = cur;
        dest++;
    }
    size_t old_size = buf->fill_size;
    if (buf_fill(fd, buf, 4096) == -1) {
        return -1;
    }
    for (i = old_size; i < buf->fill_size; i++) {
        char cur = *(buf->data + i);
        if (cur == '\n') {
            dest = '\0';
            erase_first_n(buf, i+1);
            return i;
        }
        *dest = cur;
        dest++;
    }
}

void buf_clear(struct buf_t *buf) {
    free(buf->data);
    buf->fill_size = 0;
}
