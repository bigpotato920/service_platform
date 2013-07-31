#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

#define UNIX_SERVER_PATH "/var/tmp/agent"

typedef struct {

	int next_hop;
	char state[4];

} route_state_t;

int main(int argc, char const *argv[])
{
	int client_fd;
	int addr_len;
	int nwrite;
	int nread;
	int flag;

	struct sockaddr_un server_addr;
	route_state_t route_state;

	if ((client_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("create socket");
		return -1;
	}


	memset(&server_addr, 0, sizeof(struct sockaddr_un));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, UNIX_SERVER_PATH);
	addr_len = offsetof(struct sockaddr_un, sun_path) + strlen(server_addr.sun_path);

	if (connect(client_fd, (struct sockaddr*)&server_addr, addr_len) < 0) {
		perror("socket connect");
		return 1;
	}

	route_state.next_hop = -1;
	strncpy(route_state.state, "0000", 4);
	while (1) {
		nread = read(client_fd, &flag, 1);
		printf("nread = %d\n", nread);
		if (nread == 1 && flag == 0) {
			nwrite = write(client_fd, &route_state, sizeof(route_state));
			printf("send route state to the agent\n");
		}
	}
	close(client_fd);

	return 0;
}