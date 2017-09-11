#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include "main_routine.h"

#define HELP "usage %s ACTION OPTION\nACTION:\n\t--start: start the server\n\t--kill:  kill the server\n"

int 	main(int argc, char const *argv[]) {
  FILE     *fp;
  pid_t    pid;
  int      check;
  int      check_pid;
  int      i;
  char     buffer[64];

  pid = getpid();
  check = 0;
  sprintf(buffer, "ps -ef | grep %s | grep -v grep | awk '{print $2}'", argv[0]);
  fp = popen(buffer, "r");
  if (fp == NULL) {
    perror("Popen() Fail");
    exit(EXIT_FAILURE);
  }
  fgets(buffer, sizeof(buffer)-1, fp);
  check_pid = atoi(buffer);
  pclose(fp);

  for (i = 1; i < argc; i++) {
    if(strcmp(argv[i], "--start") == 0) {
      if (pid != check_pid) {
        fprintf(stderr, "Server already started\n");
        check++;
        break;
      }
      pid = fork();
      if (pid < 0) {
        perror("Fork() Fail");
        exit(EXIT_FAILURE);
      }
      if (pid > 0) main_routine(argc,argv);
      check++;
      break;
    }
    else if(strcmp(argv[i], "--kill") == 0)  {
      pid = atoi(buffer);
      if (pid == getpid()) {
        fprintf(stderr, "Server not running\n");
        check++;
        break;
      }
      kill(pid, SIGINT);
      check++;
      break;
    }
    else if(strcmp(argv[i], "--help") == 0) {
      fprintf(stderr,  HELP, argv[0]);
      check++;
      break;
    }
  }
  if (check == 0) fprintf(stderr, "Type: %s --help\n", argv[0]);
  exit(EXIT_SUCCESS);
}
