#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "protocol.h"

#define DEBUG 1

int recv_message(int socket_desc, char* buffer,  int buffer_len, int flags) {
  int   ret;
  int   bytes_read = 0;

  while(bytes_read < buffer_len) {
    ret = recv(socket_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret <= 0) {
			if (DEBUG && ret == -1) perror("recv_message");
      if (DEBUG && ret == 0) perror("Connection closed by client");
      return -1;
    }
    bytes_read++;
	  if (buffer[bytes_read-1] == '\0' && (flags & 1) == 1) break;
    if (buffer[bytes_read-1] == '\n' && (flags & 2) == 2) break;
	  if (buffer[bytes_read-1] == '\r' && (flags & 4) == 4) break;
  }
  buffer[bytes_read-1] = '\0';
  return bytes_read;
}

int send_message(int socket_desc, char* buffer, int buffer_len, int flags) {
  int   ret;
  int   bytes_send = 0;

  while (bytes_send < buffer_len) {
    ret = send(socket_desc, buffer + bytes_send, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
	    if (DEBUG) perror("send_message");
      return -1;
    }
    bytes_send ++;
    if (buffer[bytes_send-1] == '\0' && (flags & 1) == 1) break;
    if (buffer[bytes_send-1] == '\n' && (flags & 2) == 2) break;
	  if (buffer[bytes_send-1] == '\r' && (flags & 4) == 4) break;
  }
  return bytes_send;
}
