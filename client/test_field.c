#include "../libs/common.h"
#include "../libs/client_linux_utils.h"
#include "../libs/server_protocol.h"



int main(int argc, char const *argv[]) {
  int sock_desc,ret;
  void * status;
  pthread_t t_input,t_output;


  ret = pthread_create(&t_input,NULL,mini_shell,(void *)sock_desc);
  PTHREAD_ERROR_HELPER(ret,"Errore creazione thread t_input: ");

  ret = pthread_join(t_input, &status);
  PTHREAD_ERROR_HELPER(ret,"Errore join in: ");

  printf("Bye Bye\n");

  return 0;
}
