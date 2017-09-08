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
    if (buffer[bytes_read-1] == '\n' ||
				buffer[bytes_read-1] == '\0' )
			break;
  }
  buffer[bytes_read-1] = '\0';
  return bytes_read;
}

int send_message(int socket_desc, char* buffer, int buffer_len) {
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
    if (buffer[bytes_send-1] == '\n' ||
				buffer[bytes_send-1] == '\0' )
			break;
  }
  return bytes_send;
}

int recv_query(int socket_desc, char* query,  int query_len) {
  int   ret;
  int   bytes_read = 0;

  while(bytes_read < query_len) {
    ret = recv(socket_desc, query + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret <= 0) {
			if (DEBUG && ret == -1) perror("recv_message");
      if (DEBUG && ret == 0) perror("Connection closed by client");
      return -1;
    }
    bytes_read++;
    if (query[bytes_read-1] == '\0' ) break;
  }
  query[bytes_read-1] = '\0';
  return bytes_read;
}

int send_query(int socket_desc, char* query, int query_len) {
  int   ret;
  int   bytes_send = 0;

  while (bytes_send < query_len) {
    ret = send(socket_desc, query + bytes_send, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
	    if (DEBUG) perror("send_message");
      return -1;
    }
    bytes_send ++;
    if (query[bytes_send-1] == '\0' ) break;
  }
  return bytes_send;
}
