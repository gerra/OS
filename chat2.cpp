#include <string>
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
#include <map>
#include <set>
#include <string.h>
#include <stdio.h>

using namespace std;

int setSockNonBlock(int fd) {
	int flags = fcntl(fd, F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int createServerSocket(const char *address, const char *port) {
	struct addrinfo hints;
	struct addrinfo *addresses, *rv;

	memset(&hints, 0, sizeof(hints));

	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if (getaddrinfo(address, port, &hints, &addresses) == -1) {
		exit(1);
	}
	
	int res = -1;
	for (rv = addresses; rv != NULL; rv = rv->ai_next) {
		res = socket(AF_INET, SOCK_STREAM, 0);
		if (res == -1) continue;
		int one = 1;
		if (setsockopt(res, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int))== -1) {
			close(res);
			continue;
		}
		if (bind(res, rv->ai_addr, rv->ai_addrlen) == 0) {
			break;
		}
		close(res);
	}
	if (rv == NULL) res = -1;
	freeaddrinfo(addresses);
	if (listen(res, 128) == -1) exit(1);
	setSockNonBlock(res);
	return res;
}

#define N 129

int nfds, oldnfds;
struct pollfd fds[N];
struct buf_t *bufs[N];
map<string, set<int> > roomToClients;
map<int, string> clientToRoom;
map<int, string> sockMessages;

void deleteClient(int id) {
	nfds--;
	int fd = fds[id].fd;
	close(fd);
	string &room = clientToRoom[fd];
	roomToClients[room].erase(fd);
	clientToRoom.erase(fd);
	buf_free(bufs[id]);
	bufs[id] = bufs[nfds];
	fds[id] = fds[nfds];
}

int main(int argc, char **argv) {
	if (argc != 2) {
		write(STDOUT_FILENO, "USAGE: ./chat2 <port>\n", 22);
		exit(1);
	}
	int serv = createServerSocket("0.0.0.0", argv[1]);
	
	int nfds = 1;
	fds[0].fd = serv;
	fds[0].events = POLLIN;
	
	for (;;) {
		int changed = poll(fds, nfds, -1);
		if (changed == -1) continue;
		if (fds[0].revents & POLLIN) {
			int client = accept(fds[0].fd, NULL, NULL);
			fds[nfds].fd = client;
			fds[nfds].events = POLLIN;
			bufs[nfds] = buf_new(4096);
			setSockNonBlock(client);
			nfds++;
			printf("New client on socket %d\n", client);
		}
		int oldnfds = nfds;
		int i;
		for (i = 1; i < oldnfds; i++) {
			if (fds[i].revents & POLLIN) {
				char *str = (char *) malloc(4096);
				int sz = buf_getline(fds[i].fd, bufs[i], str);
				if (sz == 0) {
					printf("EOF read, deleting client\n");
					deleteClient(i);
				} else if (clientToRoom.find(fds[i].fd) == clientToRoom.end()) {
					str[--sz] = 0;
					string room = string(str);
					clientToRoom[fds[i].fd] = room;
					roomToClients[room].insert(fds[i].fd);
					printf("Room of %d: %s (size = %d)\n", fds[i].fd, str, sz);
				} else {
					string message = string(str, sz);
					string &room = clientToRoom[fds[i].fd];
					printf("Message size = %d\n", sz);
					
					for (set<int>::iterator it = roomToClients[room].begin(); it != roomToClients[room].end(); ++it) {
						sockMessages[*it] += message;
						fds[i].events |= POLLOUT;
					}
				}
				free(str);
			} else if (fds[i].revents & POLLOUT) {
				int total_sent = 0;
				string &message = sockMessages[fds[i].fd];
				for (;;) {
					int sent_now = write(fds[i].fd, message.c_str(), message.length());
					if (sent_now <= 0) break;
					total_sent += sent_now;
					message = message.substr(sent_now);
				}
				fds[i].events |= POLLIN;
			} else if (fds[i].revents & POLLHUP) {
				deleteClient(i);
			}
		}
	}

	return 0;
}
