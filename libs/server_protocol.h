#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define DEBUG           0
#define QUERY_LEN       5
#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT     8583

#define OFFLINE   0
#define ONLINE    1

int server_status(int sock_desc, int status);
int server_connect(struct sockaddr_in* sock_addr, char* name, size_t name_len);
int download_list(int socket_desc, char* buffer, size_t buff_len);
int server_disconnect(int sock_desc);
int recv_message(int socket_desc, char* buffer,  int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);
