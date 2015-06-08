#include <stdio.h>
#include <arpa/inet.h>
#include "../lib/bufio.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>

int createBoundSocket(char *address, char *port) {
    struct addrinfo hints;
    struct addrinfo *addresses, *rv;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC; // whatever ipv4 or ipv6
    hints.ai_socktype = SOCK_STREAM; // tcp connection
    if (getaddrinfo(address, port, &hints, &addresses) == -1) {
        perror("getaddrinfo");
        exit(-1);
    }

    int resultSocket;
    for (rv = addresses; rv != NULL; rv = rv->ai_next) {
        resultSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (resultSocket == -1) {
            printf("socket error\n");
            continue;
        }
        int one = 1;
        if (setsockopt(resultSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
            printf("stsockopt error\n");
            close(resultSocket);
            continue;
        }
        if (bind(resultSocket, rv->ai_addr, rv->ai_addrlen) == 0) {
            break;
        }

        close(resultSocket);
    }
    if (rv == NULL) {
        printf("rv = NULL\n");
        resultSocket = -1;
    }
    freeaddrinfo(addresses);
    return resultSocket;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("USAGE: ./filesender <port> <filename>\n");
        exit(-1);
    }
    int sock = createBoundSocket("0.0.0.0", argv[1]);
    if (sock == -1) {
        printf("Cannot create bound socket\n");
        exit(1);
    }

    if (listen(sock, 1) == -1) {
        perror("listen");
        exit(1);
    }
    printf("%d\n", sock);
    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    while (1) {
        int fd = accept(sock, &client, &sz);
        if (fd == -1) {
            perror("accept");
            continue;
        }
        pid_t pid = fork();
        if (pid == -1) {
            close(fd);
            continue;
        } else if (pid == 0) {
            close(sock);
            int filefd = open(argv[2], O_RDONLY);
            struct buf_t *buf = buf_new(4096);
            int len;
            while ((len = buf_fill(filefd, buf, 1)) > 0) {
                buf_flush(fd, buf, len);
            }
            buf_free(buf);
            close(filefd);
            close(fd);
            exit(0);
        } else {
            close(fd);
            continue;
        }
    }
    close(sock);
    return 0;
}
