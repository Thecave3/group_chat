#include "server_protocol.h"

int server_connect(char* name) {
  int   ret;
  int   bytes_send = 0;
  char  data_buffer[PACKET_LEN];
  struct sockaddr_in* sock_addr = malloc(sizeof(struct sockaddr_in));
  int   sock_desc = socket(AF_INET,SOCK_STREAM,0);

  if (sock_desc <= 0) {
    return -1;
  }
  //memcpy (data_buffer	  ,port	, 4);
  memcpy (data_buffer/*+4*/	,name , 12);
  //fprintf(stderr, "%s\n", data_buffer);
  sock_addr->sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
  sock_addr->sin_family = AF_INET;
  sock_addr->sin_port = htons(SERVER_PORT);
  if (connect(sock_desc,(struct sockaddr*) sock_addr,sizeof(struct sockaddr_in))) {
    if (DEBUG) perror("server_connect: error in connect");
    return -1;
  }
  while (bytes_send < PACKET_LEN) {
    ret = send(sock_desc, data_buffer + bytes_send, PACKET_LEN - bytes_send, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      if (DEBUG) perror("server_connect: error in send");
      return -1;
    }
    bytes_send += ret;
  }
  return sock_desc;
}

int server_status (int sock_desc, int status) {
  int         ret;
  int         query_send = 0;
  char        query[5];

  if (status == ONLINE) {
      query[0] = 'S';
      query[1] = 'T';
      query[2] = 'O';
      query[3] = 'N';
      query[4] = '\0';
  }
  if (status == OFFLINE) {
    query[0] = 'S';
    query[1] = 'T';
    query[2] = 'O';
    query[3] = 'F';
    query[4] = '\0';
  }

  while (query_send < QUERY_LEN) {
    ret = send(sock_desc, query + query_send, QUERY_LEN - query_send, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      if (DEBUG) perror("server_status: error in send");
      return -1;
    }
    query_send += ret;
  }
  return 1;
}

int download_list(int sock_desc, char* buffer, size_t buff_len) {
  int         ret;
  int         query_send = 0;
  int         bytes_read = 0;
  char        query[5] = {'L','I','S','T','\0'};

  while (query_send < QUERY_LEN) {
    ret = send(sock_desc, query + query_send, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      if (DEBUG) perror("download_list: error in (query) send");
      return -1;
    }
    query_send ++;
  }

/*  while(1) {
    ret = recv(sock_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      if (DEBUG) perror("download_list: error in recv");
      return -1;
    }
    if (ret == 0) {
      if (DEBUG) perror("download_list: connection closed by client");
      return 0;
    }
    bytes_read++;
    if (buffer[bytes_read-1] ==  '\0') break;
  }
  return bytes_read;*/
  return 1;
}

int server_disconnect(int sock_desc) {
  int         ret;
  int         query_send = 0;
  char        query[5] = {'Q','U','I','T','\0'};

  while (query_send < QUERY_LEN) {
    ret = send(sock_desc, query + query_send, QUERY_LEN - query_send, 0);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      if (DEBUG) perror("server_disconnect: error in send");
      return -1;
    }
    query_send += ret;
  }
  return 1;
}

int recv_message(int socket_desc, char* buffer,  int buffer_len) {
  int   ret;
  int   bytes_read = 0;

  while(1) {
    ret = recv(socket_desc, buffer + bytes_read, 1, 0);
    if (ret == -1 && errno == EINTR)
			continue;
    if (ret == -1) {
			if (DEBUG) perror("recv_message: error in recv");
      return -1;
    }
    if (ret == 0) {
			if (DEBUG) perror("recv_message: connection closed by client");
      return 0;
    }
    bytes_read++;
    if (buffer[bytes_read-1] == '\n' ||
				buffer[bytes_read-1] ==  '\0' ||
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
			if (DEBUG) perror("send_message: error in send");
      return -1;
    }
    bytes_send += ret;
  }
  return bytes_send;
}


int connect_to(int sock_desc,int client_id){

  return 0;
}
