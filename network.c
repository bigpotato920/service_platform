#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#include "network.h"
#define QLEN 1

int create_udp_server(const char *ip, unsigned int port)
{
	int server_fd;
	struct sockaddr_in server_addr;

	if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("socket error");
		return -1;
	}

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);

	if (bind(server_fd, (struct sockaddr*)&server_addr, 
		sizeof(server_addr)) < 0) {
		perror("bind error");
		return -1;
	}

	return server_fd;
}


/**
 * craete a server in unix domain
 * @param  path_name path name of the socket address
 * @return           server socket descriptor or -1 on failure
 */
int create_unix_server(const char* path_name) 
{
	struct sockaddr_un server_addr;
	size_t addr_len;
	int server_fd;

	if ((server_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("create socket");
		return -1;
	}
	unlink(path_name);

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sun_family = AF_UNIX;
	strcpy(server_addr.sun_path, path_name);
	addr_len = offsetof(struct sockaddr_un, sun_path) + strlen(path_name);

	if (bind(server_fd, (struct sockaddr*)&server_addr, addr_len) < 0) {
		perror("socket bind");
		return -1;
	}

	if (listen(server_fd, QLEN) < 0) {
		perror("socket listen");
		return -1;
	}

	return server_fd;
}

/**
 * Server accept a connection from the client
 * @param  server_fd server socket descriptor
 * @return           client socket descriptor
 */
int unix_server_accept(int server_fd)
{
	int client_fd;
	socklen_t addr_len;
	struct sockaddr_un client_addr;

	addr_len = sizeof(client_addr);

	if ((client_fd = accept(server_fd, (struct sockaddr*)&client_addr
		, &addr_len) )< 0) {
		perror("socket accept");
	}

	return client_fd;
}

/**
 * receive message from the udp client
 * @param  server_fd server socket descriptor
 * @param  msg       message 
 * @param  len       message length
 * @return           bytes received
 */
int recvfrom_udp_client(int server_fd, void *msg, int len) 
{

	int nread;
	
	nread = read(server_fd, msg, len);

	return nread;
}

/**
 * Send msg to specific udp server
 * @param  ip  ip address
 * @param port server port
 * @param  msg message to be send
 * @param len message length
 * @return    bytes send on successor -1 on failure
 */
int sendto_udp_server(const char *ip, int port, void *msg, int len) 
{

	int sock_fd;
	int nsend;
	struct sockaddr_in server_addr;
	socklen_t sock_len;

	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket error");
		return -1;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);
	sock_len = sizeof(struct sockaddr_in);

	nsend = sendto(sock_fd, msg, len, 0, (struct sockaddr*)&server_addr, sock_len);

	return nsend;
}