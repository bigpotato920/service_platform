#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#include "network.h"

#define ROUTE_CONFIG "Net_config"
#define AGENT_CONFIG "agent.ini"

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


typedef struct {

	int next_hop;
	char state[];

} route_state_t;

typedef struct {

	collect_state_t collect_state;
	route_state_t route_state;

} state_t;


state_t state_msg;    
char agent_server_path[BUFSIZ];
char udp_server_ip[BUFSIZ];
int udp_server_port;
char main_server_ip[BUFSIZ];
int main_server_port;
int slot_num;
int unix_clinet_fd;

void read_config();
int request_route_state(int unix_server_fd) ;
void start_agent_service(int unix_server_fd, int udp_server_fd);
int sendto_main_server(char *server_ip, int server_port);


void test_config() {
	printf("agent_server_path = %s\n", agent_server_path);
	printf("udp server ip = %s\n", udp_server_ip);
	printf("udp server port = %d\n", udp_server_port);
	printf("main_server_ip = %s\n", main_server_ip);
	printf("main_server_port = %d\n", main_server_port);
	printf("slot num = %d\n", slot_num);
}
/**
 * read the configuration from the file
 * @param filename configuration filename
 */
void read_config()
{
	FILE *fp = fopen(ROUTE_CONFIG, "r");
	char key[BUFSIZ];
	char val[BUFSIZ];

	while ((fscanf(fp, "%7s=%s", key, val)) == 2) {
		if (strcmp(key, "PortNum") == 0) {
			sscanf(val, "%d", &slot_num);
			break;
		} 
		printf("key = %s, val = %s\n", key, val);
	}
	fclose(fp);

	fp = fopen(AGENT_CONFIG, "r");

	while ((fscanf(fp, "%s %s", key, val)) == 2) {
		if (strcmp(key, "AGENT_SERVER_PATH") == 0) {
			strcpy(agent_server_path, val);
		} else if (strcmp(key, "UDP_SERVER_IP") == 0) {
			strcpy(udp_server_ip, val);
		} else if (strcmp(key, "UDP_SERVER_PORT") == 0) {
			sscanf(val, "%d", &udp_server_port);
		} else if (strcmp(key, "MAIN_SERVER_IP") == 0) {
			strcpy(main_server_ip, val);
		} else if (strcmp(key, "MAIN_SERVER_PORT") == 0) {
			sscanf(val, "%d", &main_server_port);
		}
	}
	fclose(fp);

}

/**
 * send to state message to the main server
 * @param  server_ip   server ip
 * @param  server_port server port
 * @return             bytes send on success or -1 on failure
 */
int sendto_main_server(char *server_ip, int server_port)
{
	int rv;

	printf("send to main server\n");
	rv = sendto_udp_server(server_ip, server_port, &state_msg, sizeof(state_t) + slot_num);

	return rv;
}

/**
 * send request for the route and state of the slots to the controller
 * @param  unix_server_fd unix server socket descriptor
 * @return                0 on success or -1 on failure
 */
int request_route_state(int unix_server_fd) 
{
	int nwrite;
	int flag = 0;

	nwrite = write(unix_server_fd, &flag, 1);
	printf("request route message\n");
	if (nwrite == 1)
		return 0;
	else
		return -1;
}


/**
 * start the agent service 
 * @param unix_server_fd [description]
 * @param udp_server_fd  [description]
 */
void start_agent_service(int unix_server_fd, int udp_server_fd)
{
	int unix_client_fd;
    int nread;
    int tmp_fd;
    int rv;

    fd_set readfds;
    fd_set testfds;

    FD_ZERO(&readfds);
    FD_SET(unix_server_fd, &readfds);
    FD_SET(udp_server_fd, &readfds);

    while (1) {

        testfds = readfds;
        rv = select(FD_SETSIZE, &testfds, (fd_set *)NULL, (fd_set *)NULL, (struct timeval *)NULL);
        switch (rv)
        {
    
        case -1:
            perror("select");
            break;
        default:

        	for (tmp_fd = 0; tmp_fd < FD_SETSIZE; tmp_fd++) {
        		if (FD_ISSET(tmp_fd, &testfds)) {

        			if (tmp_fd == unix_server_fd) {
            			unix_client_fd = unix_server_accept(unix_server_fd);
            			printf("a client connected\n");
            			FD_SET(unix_client_fd, &readfds);
            	
            		} else if (tmp_fd == udp_server_fd) {
            			memset(&state_msg, 0, sizeof(state_msg) + slot_num);
            			nread = recvfrom_udp_client(udp_server_fd, &state_msg, 
            				sizeof(collect_state_t));
            			printf("nread = %d\n", nread);
            			request_route_state(unix_client_fd);

            		} else {
            			nread = read(tmp_fd, &state_msg+sizeof(collect_state_t), 
            				sizeof(route_state_t) + slot_num);
            			if (nread > 0) {
         					sendto_main_server(main_server_ip, main_server_port);
       
            			} else if (nread == 0) {
            				printf("a client disconnected\n");
            				FD_CLR(tmp_fd, &readfds);
            				close(tmp_fd);
            			} else {
            				FD_CLR(tmp_fd, &readfds);
            				close(tmp_fd);
            			}
            		}
        		}
	
        	}
        	break;
        }
        	
    }
}


int main(int argc, char const *argv[])
{
	int udp_server_fd;
	int unix_server_fd;

	read_config();
	test_config();
	
	unix_server_fd = create_unix_server(agent_server_path);
	udp_server_fd = create_udp_server(udp_server_ip, udp_server_port);

	printf("%d, %d\n", unix_server_fd, udp_server_fd);
	start_agent_service(unix_server_fd, udp_server_fd);
	
	close(unix_server_fd);
	close(udp_server_fd);

	return 0;
}