#include "server_protocol.h"
#include "server_utils.h"

int recv_message(int socket_desc, char* buffer,  int buffer_len) {
  int   ret;
  int   bytes_read = 0;

  signal(SIGINT, exit);

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

int send_list(int socket_desc) {
	client_l aux = client_list;
	char buffer[84*nclients];
	while (aux != NULL) {
		strcpy(buffer, aux->client_name);
		strcat(buffer, "\\n");
		strcat(buffer, aux->client_ip);
		strcat(buffer, "\\n\\r");
		aux = aux->next;
    if (DEBUG) printf("%s\n", buffer);
	}
	if (send_message(socket_desc, buffer, sizeof(buffer))){
		if (DEBUG) fprintf(stderr, "\tsend_list\n");
		return 0;
	}
	return 1;
}
