#include <stdio.h>
#include <arpa/inet.h>
#include "../lib/bufio.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <wait.h>
#include <poll.h>
#include <errno.h>

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
        return -1;
    }

    if (listen(sock, 128) == -1) {
        return -1;
    }

    return sock;
}

const int PAIRS = 127;
const int N = 127 * 2 + 2;

struct buf_t *bufs[127 * 2 + 2][2];
struct pollfd fds[127 * 2 + 2];
int disconnected[127 * 2 + 2];
int oldnfds;
int nfds;
int accepting;

void deleteOne(int i, int f) {
    close(fds[i].fd);
    fds[i].fd = fds[f].fd;
    fds[i].events = fds[f].events;
    bufs[i/2][0] = bufs[f/2][0];
    bufs[i/2][1] = bufs[f/2][1];
    disconnected[i] = disconnected[f];
    disconnected[i ^ 1] = disconnected[f ^ 1];
    nfds--;
}

void deletePair(int i) {
    printf("deleting %d, cnt = %d...\n", i, oldnfds);
    int f;
    if (i & 1) {
        f = oldnfds - 1;
    } else {
        f = oldnfds - 2;
    }

    if (oldnfds & 1) {
        if (i < i ^ 1) {
            deleteOne(i, f);
        } else {
            deleteOne(i ^ 1, f ^ 1);
        }
        accepting = 0;
        fds[0].events = POLLIN;
        fds[1].events = 0;
    } else {
        deleteOne(i, f);
        deleteOne(i ^ 1, f ^ 1);
    }


}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("USAGE: ./polling <port1> <port2>\n");
        exit(1);
    }
    int srv[2] = {
        createServerSocket("0.0.0.0", argv[1]),
        createServerSocket("0.0.0.0", argv[2])
      //  createServerSocket("127.0.0.1", argv[1]),
      //  createServerSocket("127.0.0.1", argv[2])
    };
    if (srv[0] == -1 || srv[1] == -1) {
        printf("Cannot create bond sockets and start listening (%d %d)\n", srv[0], srv[1]);
    }

    printf("Server1: %d\n", srv[0]);
    printf("Server2: %d\n", srv[1]);


    int i;
    for (i = 0; i < PAIRS; i++) {
        int j;
        for (j = 0; j < 2; j++) {
            bufs[i][j] = buf_new(4096);
            if (bufs[i][j] == NULL) {
                printf("Buffers cannot be created\n");
                exit(1);
            }
        }
    }


    nfds = 2;
    fds[0].fd = srv[0];
    fds[0].events = POLLIN;
    fds[1].fd = srv[1];
    fds[1].events = 0;

    accepting = 0;

    //memset(&disconnected, 0, sizeof(int));
    while (1) {
        int changed = poll(fds, nfds, -1);
       // printf("Changed count: %d\n", changed);
        if (changed == -1) {
            if (errno != EINTR) {
                perror("poll");
                exit(1);
            }  else {
                continue;
            }
        }
        oldnfds = nfds;
        if (accepting == 0 && (fds[0].revents & POLLIN)) {
            if (nfds + 2 < N) {
                printf("Accepted on 1st: ");
                int newClient = accept(fds[0].fd, NULL, NULL);
                printf("%d\n", newClient);
                fds[nfds].fd = newClient;
                fds[nfds].events = POLLIN;
                disconnected[nfds] = 0;
                disconnected[nfds+1] = 1;
                nfds++;
                fds[0].events = 0;
                fds[1].events = POLLIN;
                accepting = 1;
            }
        }
        if (accepting == 1 && (fds[1].revents & POLLIN)) {
            if (nfds + 1 < N) {
                    printf("Accepted on 2nd: ");
                    int newClient = accept(fds[1].fd, NULL, NULL);
                    printf("%d\n", newClient);
                    fds[nfds].fd = newClient;
                    fds[nfds].events = POLLIN;
                    disconnected[nfds] = 0;
                    nfds++;
                    fds[0].events = POLLIN;
                    fds[1].events = 0;
                    accepting = 0;
                }
        }

        for (i = 2; i < oldnfds; i++) {
            int bid1 = i / 2;
            int bid2 = i % 2;
            if (fds[i].revents & POLLHUP) {
                printf("    changed id: %d (POLLHUP)\n", fds[i].fd);

                deletePair(i);

            } else if (fds[i].revents & POLLIN) {
              //  printf("    changed sock: %d (POLLIN)\n", fds[i].fd);
               // printf("    i = %d\n", i);
                int len = buf_fill(fds[i].fd, bufs[bid1][bid2], 1);
               // printf("   read: %d\n", len);
                if (len <= 0) {
                    shutdown(fds[i].fd, SHUT_RD);
                    fds[i].events = 0;
                    disconnected[i] = 1;
                    if (disconnected[i ^ 1] == 1) {
                        deletePair(i);
                    }
                } else {
                    fds[i ^ 1].events |= POLLOUT;
                }
            }  else if (fds[i].revents & POLLOUT) {
               // printf("    changed id: %d (POLLOUT)\n", fds[i].fd);
                int len = buf_flush(fds[i].fd, bufs[bid1][bid2 ^ 1], buf_size(bufs[bid1][bid2 ^ 1]));
                fds[i].events = POLLIN;
                if (len <= 0) {
                    shutdown(fds[i].fd, SHUT_WR);
                }
            }
        }
    }

    return 0;
}

