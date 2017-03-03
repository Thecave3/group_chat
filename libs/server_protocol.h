#include "common.h"

#define OFFLINE 0
#define ONLINE  1

int server_status(int sock_desc, int status);
int server_connect(struct sockaddr_in* sock_addr, char* name, size_t name_len);
int download_list(int socket_desc, char* buffer, size_t buff_len);
int server_disconnect(int sock_desc);
int recv_message(int socket_desc, char* buffer,  int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);
