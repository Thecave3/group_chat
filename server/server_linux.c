#include "server_header.h"

int 	main(int argc, char const *argv[]) {
  FILE *fp;
  pid_t pid;
	int check;
	int check_pid;
  char buffer[1024];
  pid = getpid();
  check = 0;

	sprintf(buffer, "ps -ef | grep %s | grep -v grep | awk '{print $2}'", argv[0]);
	fp = popen(buffer, "r");
	if (fp == NULL) {
		perror("popen");
		exit(EXIT_FAILURE);
	}
	fgets(buffer, sizeof(buffer)-1, fp);
	check_pid = atoi(buffer);
	pclose(fp);
	for (int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--start") == 0) {
			if (pid != check_pid) {
				fprintf(stderr, "Server already started\n");
				check++;
				break;
			}
			pid = fork();
			if (pid < 0) {
				perror("fork");
				exit(EXIT_FAILURE);
			}
			if (pid > 0) server_routine(argc,argv);
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
		else if(strcmp(argv[i], "-l") == 0) continue;
		else if(strcmp(argv[i], "-ld") == 0) continue;
		else if(strcmp(argv[i], "--help") == 0) {
			fprintf(stderr,  "usage %s ACTION OPTION\nACTION:\n\t--start: start the server\n\t--kill:  kill the server\nOPTION:\n\t-l:  enable log system\n\t-ld: enable log system in debug mode\n", argv[0]);
			check++;
			break;
		}
	}
	if (check == 0) fprintf(stderr, "Type: %s --help\n", argv[0]);
	exit(EXIT_SUCCESS);
}
