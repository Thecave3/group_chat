#ifndef LOGGER_H
#define LOGGER_H

#include <semaphore.h>

#define PATH_LEN 128

typedef struct logger_s {
  int descriptor;
  sem_t semaphore;
  char path[PATH_LEN];
} logger_c;
typedef logger_c* logger_t;

logger_t new_logger(char *name, char* folder);
int write_logger(logger_t logger, const char* format, ...);
void close_logger(logger_t logger);

#endif
