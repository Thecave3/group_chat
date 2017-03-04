#include <stdio.h>
#include <fcntl.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <semaphore.h>

#define DEBUG 1

typedef struct logger_s {
  sem_t log_semaphore;
  FILE* file_desc;
  char  path[256];
} logger_t;

logger_t* new_log (const char* path);
int       write_log (logger_t* logger,const char* str, ...);
int       destroy_log (logger_t* logger);
