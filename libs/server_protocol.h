#include "common.h"


int recv_message(int socket_desc, char* buffer,  int buffer_len);
int send_message(int socket_desc, char* buffer, int buffer_len);
int send_list(int socket_desc);
