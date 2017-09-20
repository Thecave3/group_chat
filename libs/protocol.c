#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"

#define MAX_CONN_QUEUE 10

void server_init(int* sock_desc, struct sockaddr_in* sock_addr) {
  *sock_desc = socket(AF_INET , SOCK_STREAM , 0);
  if (*sock_desc == -1) exit(EXIT_FAILURE);
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_addr.s_addr = INADDR_ANY;
  sock_addr->sin_port = htons(SERVER_PORT);
  if (bind(*sock_desc,(struct sockaddr *)sock_addr ,sizeof(*sock_addr)) < 0) exit(EXIT_FAILURE);
  if(listen(*sock_desc , MAX_CONN_QUEUE)) exit(EXIT_FAILURE);
}

int recv_message(int socket_desc, char* buffer,  int buffer_len, int flags) {
  int   ret;
  int   bytes_read = 0;

  while(bytes_read < buffer_len) {
    ret = recv(socket_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret == -1) return -1;
    bytes_read++;
	  if (buffer[bytes_read-1] == '\0' && (flags & 1) == 1) break;
    if (buffer[bytes_read-1] == '\n' && (flags & 2) == 2) break;
	  if (buffer[bytes_read-1] == '\r' && (flags & 4) == 4) break;
  }
  return bytes_read;
}

int send_message(int socket_desc, char* buffer, int buffer_len, int flags) {
  int   ret;
  int   bytes_send = 0;

  while (bytes_send < buffer_len) {
    ret = send(socket_desc, buffer + bytes_send, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) return -1;
    bytes_send ++;
    if (buffer[bytes_send-1] == '\0' && (flags & 1) == 1) break;
    if (buffer[bytes_send-1] == '\n' && (flags & 2) == 2) break;
	  if (buffer[bytes_send-1] == '\r' && (flags & 4) == 4) break;
  }
  return bytes_send;
}
