#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"

#define DEBUG 1

int recv_message(int socket_desc, char* buffer,  int buffer_len) {
  int   ret;
  int   bytes_read = 0;

  while(1) {
    ret = recv(socket_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret == -1) {
			if (DEBUG) perror("recv_message");
      return -1;
    }
    if (ret == 0) {
			if (DEBUG) perror("Connection closed by client");
      return 0;
    }
    bytes_read++;
    if (buffer[bytes_read-1] == '\n' ||
				buffer[bytes_read-1] == '\0' ||
				bytes_read == buffer_len)
			break;
  }
  buffer[bytes_read-1] = '\0';
  return bytes_read;
}

int send_message(int socket_desc, char* buffer, int buffer_len) {
  int   ret;
  int   bytes_send = 0;

  while (bytes_send < buffer_len) {
    ret = send(socket_desc, buffer + bytes_send, buffer_len - bytes_send, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
	  if (DEBUG) perror("send_message");
      return -1;
    }
    bytes_send += ret;
  }
  return bytes_send;
}
