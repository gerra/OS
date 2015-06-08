#include <stdio.h>
#include <arpa/inet.h>
#include "../lib/bufio.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <wait.h>

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
            continue;
        }
        int one = 1;
        if (setsockopt(resultSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
            close(resultSocket);
            continue;
        }
        if (bind(resultSocket, rv->ai_addr, rv->ai_addrlen) == 0) {
            break;
        }
        close(resultSocket);
    }
    if (rv == NULL) {
        resultSocket = -1;
    }
    freeaddrinfo(addresses);
    return resultSocket;
}

int createServerSocket(char *address, char *port) {
    int sock = createBoundSocket(address, port);
    if (sock == -1) {
        printf("Cannot create bound socket\n");
        exit(1);
    }

    if (listen(sock, 128) == -1) {
        perror("listen");
        exit(1);
    }

    return sock;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("USAGE: ./forking <port1> <port2>\n");
        exit(1);
    }

    int sock1 = createServerSocket("0.0.0.0", argv[1]);
    int sock2 = createServerSocket("0.0.0.0", argv[2]);

    printf("server1: %d\n", sock1);
    printf("server2: %d\n", sock2);

    struct sockaddr_in client;
    socklen_t sz = sizeof(client);
    while (1) {
        int fd1 = accept(sock1, &client, &sz);
        if (fd1 == -1) {
            perror("accept");
            continue;
        }
        int fd2 = accept(sock2, &client, &sz);
        if (fd2 == -1) {
            close(fd1);
            perror("accept");
            continue;
        }

        pid_t pid1 = fork();
        if (pid1 == -1) {
            close(fd1);
            close(fd2);
            continue;
        } else if (pid1 == 0) {
            close(sock1);
            close(sock2);
            // first child
            printf("accept1: %d\n", fd1);
            struct buf_t *buf = buf_new(4096);

            while (1) {
                    int len;
                    len = buf_fill(fd1, buf, 1);
                    if (len <= 0) break;
                    len = buf_flush(fd2, buf, len);
                    if (len == -1) break;
                }

            buf_free(buf);
            close(fd1);
            exit(1);
        } else {
            pid_t pid2 = fork();
            if (pid2 == -1) {
                kill(pid1, SIGKILL);
                close(fd1);
                close(fd2);
                continue;
            } else if (pid2 == 0) {
                close(sock1);
                close(sock2);
                // second child
                printf("accept2: %d\n", fd2);
                struct buf_t *buf = buf_new(4096);

                while (1) {
                    int len;
                    len = buf_fill(fd2, buf, 1);
                    if (len <= 0) break;
                    len = buf_flush(fd1, buf, len);
                    if (len == -1) break;
                }

                buf_free(buf);
                close(fd2);
                exit(1);
            } else {
                close(fd1);
                close(fd2);
                continue;
            }
        }
    }
    close(sock1);
    close(sock2);
    return 0;
}
