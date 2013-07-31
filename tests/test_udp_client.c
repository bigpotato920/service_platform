#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>

#define UDP_SERVER_IP "127.0.0.1"
#define UDP_SERVER_PORT 8888


typedef struct {

	int nodenum;          
	long ip;   
	int video_exist;          
	int flag_gps;     
	int flag_ip;           
	int flag_serial;             
	int flag_video;          
	double x;         	
	double y; 

} collect_state_t;

int main(int argc, char const *argv[])
{
	int sock_fd;
	struct sockaddr_in server_addr;
	socklen_t sock_len;
	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket error");
		return -1;
	}

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(UDP_SERVER_PORT);
	server_addr.sin_addr.s_addr = inet_addr(UDP_SERVER_IP);
	sock_len = sizeof(struct sockaddr_in);

	collect_state_t collect_state;
	collect_state.nodenum = 0;
	collect_state.ip = 1688381632;
	collect_state.video_exist = 1;
	collect_state.flag_gps = 1;
	collect_state.flag_ip = 1;
	collect_state.flag_serial = 1;
	collect_state.flag_video = 0;
	collect_state.x = 116.351;
	collect_state.y = 39.959;

	while (1) {
		sendto(sock_fd, &collect_state, sizeof(collect_state), 0, 
			(struct sockaddr*)&server_addr, sock_len);
		printf("udp client send collecter's state message\n");
		sleep(20);
	}

	return 0;
}