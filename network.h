#ifndef NETWORK_H
#define NETWORK_H

int create_unix_server(const char* path_name);
int unix_server_accept(int server_fd);
int recvfrom_udp_client(int server_fd, void *msg, int len);
int create_udp_server(const char *ip, unsigned int port);
int sendto_udp_server(const char *ip, int port, void *msg, int len);

#endif