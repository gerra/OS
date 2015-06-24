#include "lib/bufio.h"
#include "lib/bufio.c"
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <wait.h>
#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int createBoundSocket(char *address, char *port) {
	struct addrinfo *addresses, *rv;
	
	struct addrinfo hints;
	memset(&hints,  0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(address, port, &hints, &addresses) == -1) {
		printf("getaddrinfo error\n");
		return -1;
	}
	
	int res = -1;
	for (rv = addresses; rv != NULL; rv = rv->ai_next) {
		res = socket(AF_INET, SOCK_STREAM, 0);
		if (res == -1) continue;
		int one = 1;
		if (setsockopt(res, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int)) == -1) {
			close(res);
			continue;
		}
		if (bind(res, rv->ai_addr, rv->ai_addrlen) == 0) {
			break;
		}
		close(res);
	}
	if (rv == NULL) {
		res = -1;
	}
	freeaddrinfo(addresses);
	return res;
}

int createServerSocket(char *address, char *port) {
	int res = createBoundSocket(address, port);
	if (res == -1) {
		return -1;
	}
	if (listen(res, 128) == -1) {
		printf("listen error\n");
		return -1;
	}
	return res;
}

struct buf_t *bufs[256];
int nameGot[256] ;
char names[256][4096];
int nfds, oldnfds;
struct pollfd fds[256];

void deleteClient(int i) {
	printf("Deleting %d (i = %d)\n", fds[i].fd, i);
	nfds--;
	close(fds[i].fd);
	bufs[i] = bufs[nfds];
	nameGot[i] = nameGot[nfds];
	memcpy(names[i], names[nfds], strlen(names[nfds]) + 1);
	fds[i].events = fds[nfds].events;
	fds[i].fd = fds[nfds].fd;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		printf("USAGE: ./chat <port>\n");
		exit(EXIT_FAILURE);
	}
	int server = createServerSocket(NULL, argv[1]);
	if (server == -1) {
		printf("Cannot create server socket\n");
		exit(EXIT_FAILURE);
	}
	nfds = 1;
	fds[0].events = POLLIN;
	fds[0].fd = server;

	int ii;
	for (ii = 0; ii < 256; ii++) {
		bufs[ii] = buf_new(4096);
	}
	for (;;) {
		int changed = poll(fds, nfds, -1);
		if (fds[0].revents & POLLIN) {
			printf("New client arrived\n");
			int newClient = accept(fds[0].fd, NULL, NULL);
			fds[nfds].events = POLLIN;
			fds[nfds].fd = newClient;
			nameGot[nfds] = 0;
			nfds++;
		}
		oldnfds = nfds;
		int i;
		for (i = 1; i < oldnfds; i++) {
			if (fds[i].revents & POLLIN) {
				
				int len = buf_fill(fds[i].fd, bufs[i], 1);
				printf("read %d from %d\n", len, fds[i].fd);
				if (len <= 0) {
//					deleteClient(i);
					shutdown(fds[i].fd, SHUT_RD);
					fds[i].events = 0;
				} else {
					if (nameGot[i] == 0) {
						buf_getline(fds[i].fd, bufs[i], names[i]);
						printf("Name of %d: %s\n", fds[i].fd, names[i]);
						nameGot[i] = 1;
					} else {
						int j;
						int nameLen = strlen(names[i]);
						int messageLen = buf_size(bufs[i]) + 2 + nameLen;
						char message[messageLen];
						memcpy(message, names[i], nameLen);
						message[nameLen] = ':';
						message[nameLen+1] = ' ';
						memcpy(message + nameLen + 2, bufs[i]->data, buf_size(bufs[i]));
						buf_clear(bufs[i]);
						bufs[i] = buf_new(4096);
						for (j = 1; j < nfds; j++) {
//							buf_flush(fds[j].fd, bufs[i], len);
							write(fds[j].fd, message, messageLen);
						}
					}
				}

//				buf_fill(fds[i].fd, bufs[i], )
			} else if (fds[i].revents & POLLHUP) {
				deleteClient(i);
			}
		}
	}
	return 0;
}

