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
  int   fd;
  sem_t ls;
} logger_t;

logger_t* new_log (const char* path, int flags, mode_t mode);
int       write_log (logger_t* logger, const char* str, ...);
int       destroy_log (logger_t* logger);
