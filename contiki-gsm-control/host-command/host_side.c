#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>

#define PRINT_LLADDR(addr) printf(" %02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])

#define PRINTS_LLADDR(str, addr) sprintf(str, " %02x%02x:%02x%02x:%02x%02x:%02x%02x ", ((uint8_t *)addr)[0], ((uint8_t *)addr)[1], ((uint8_t *)addr)[2], ((uint8_t *)addr)[3], ((uint8_t *)addr)[4], ((uint8_t *)addr)[5], ((uint8_t *)addr)[6], ((uint8_t *)addr)[7])

struct diag_packet {
	char msg[128];
} diag_pkt;

char *msg;

char *get_formatted_time(char *format);
struct in6_addr last_addr;
char ip_str[INET6_ADDRSTRLEN];
char ip_str1[INET6_ADDRSTRLEN];

unsigned short at_command;

enum cmd{
SIM_INIT,
SIM_CONNECT,
SIM_DATA_CONNECT, /* Got IP address */
SIM_PING,
SIM_UDP_CONNECT,
SIM_UDP_SEND,
SIM_UDP_CLOSE,
GET_CURRENT_STATE,
SIM_RESET
}command;

process_data(char *buffer, int len, FILE * fp_log)
{
int i;
	struct diag_packet *diag;
        diag = (struct diag_packet *)buffer;

	printf("App data %s\n", diag);
}


//GET FORMATED TIME
char *get_formatted_time(char *format)
{
	time_t t;
	struct tm *tmp;
	static char time_str[256];

	t = time(NULL);
	tmp = localtime(&t);
	if (tmp == NULL) {
		perror("localtime");
		exit(EXIT_FAILURE);
	}

	if (strftime(time_str, 256, format, tmp) == 0) {
		printf("strftime returned 0");
		exit(EXIT_FAILURE);
	}

	return time_str;

}

int main(int argc, char *argv[])
{
	int socket_handle, gsm_socket_handle;
	int max_server_handle = 0;
	struct sockaddr_in6 remote_address;
	struct sockaddr_in6 local_address;
	unsigned char buf[sizeof(struct in6_addr)];
	socklen_t remote_length;
	char buffer[1000];
	fd_set read_handles;
	struct timeval timeout_interval;
	int bytes_received;
	int port_number = 4000;
	int retval;
	int i;
        enum cmd command;
	struct diag_packet *remote_data;
	FILE *fp_log;

	printf("UDP APPLICATION\n");

	if (argc < 2) {
		printf ("Usage host_side_control <rem_ipv6_addr> <at_command>\n");
		exit(0);
	}
	socket_handle = socket(PF_INET6, SOCK_DGRAM, 0);
	if (socket_handle < 0) {
		perror("Unable to create socket.");
		return -1;
	}

	gsm_socket_handle = socket(PF_INET6, SOCK_DGRAM, 0);
	if (gsm_socket_handle < 0) {
		perror("Unable to create socket.");
		return -1;
	}

	memset(&local_address, 0, sizeof(local_address));

	local_address.sin6_family = AF_INET6;
	local_address.sin6_addr = in6addr_any;
	local_address.sin6_port = htons(4345);	/* host side port */

	if (bind
	    (socket_handle, (struct sockaddr *)&local_address,
	     sizeof(local_address)) < 0) {
		perror("Unable to bind.");
		return -1;
	}


	memset(&remote_address, 0, sizeof(remote_address));
	remote_address.sin6_family = AF_INET6;
	remote_address.sin6_port = htons(4343);

	printf("%s\n", argv[1]);

	if (inet_pton(AF_INET6, argv[1], &remote_address.sin6_addr) != 1) {
		printf("inet_pton... failed\n");
		exit(0);
	}

        command = htons(atoi(argv[2]));
        at_command = command;
        printf("%d \n", at_command);

	remote_length = sizeof(remote_address);
	retval = sendto(socket_handle, &at_command, sizeof(at_command), 0,
			(struct sockaddr *)&remote_address, remote_length);
	if (retval < 0)
		perror("sendto error");

	sprintf(buffer, "log_%s", get_formatted_time("%c"));

	if (!(fp_log = fopen(buffer, "a"))) {
		perror("File Error");
		exit(0);
	}

	while (1) {

		FD_ZERO(&read_handles);
		FD_SET(socket_handle, &read_handles);

		timeout_interval.tv_sec = 30;
		timeout_interval.tv_usec = 500000;

		max_server_handle = socket_handle;

		retval =
		    select(max_server_handle + 1, &read_handles, NULL, NULL,
			   &timeout_interval);
		if (retval == -1) {
			printf("Select error\n");
		} else if (retval == 0) {
			printf("No Response\n");
		} else {
			if (FD_ISSET(socket_handle, &read_handles)) {
                                //printf ("Got a packet on !\n");
				remote_length = sizeof(remote_address);

				if ((bytes_received =
				     recvfrom(socket_handle, buffer,
					      sizeof(buffer), 0,
					      (struct sockaddr *)
					      &local_address,
					      &remote_length)) < 0) {
					perror("Error in recvfrom.");
				printf("Received %d %s\n", bytes_received,buffer);
					break;
				}

				memcpy(&last_addr.s6_addr,
				       (struct sockaddr *)&remote_address, 16);
                                process_data(buffer, bytes_received, fp_log);

			}
				printf("Received %d \n",
				       bytes_received);
				memcpy(&last_addr.s6_addr,
				       &remote_address.sin6_addr, 16);

			}

		}
}
