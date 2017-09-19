#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "logger.h"

#define PATH "tmp"

logger_t new_logger(char *name, char* folder) {
  int timeStringSize;
  time_t current_time;
  struct tm * time_info;
  char timeString[64];
  struct stat st = {0};
  logger_t logger = malloc(sizeof(logger_c));

  time(&current_time);
  time_info = localtime(&current_time);
  memset(logger->path, 0, PATH_LEN);
  if (name == NULL) return NULL;
  if (folder != NULL) {
    strncat(logger->path, folder, PATH_LEN);
    if (stat(logger->path, &st) == -1) {
      if(mkdir(logger->path, 0700) == -1) return NULL;
    }
  }
  strncat(logger->path, "/",PATH_LEN);
  strncat(logger->path, name, PATH_LEN);
  strncat(logger->path, " - ",PATH_LEN);
  strftime(timeString, 64, "%c", time_info);
  strncat(logger->path, timeString, PATH_LEN);
  if (sem_init(&logger->semaphore, 0, 1) == -1) {
    free(logger);
    return NULL;
  }
  logger->descriptor = open(logger->path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if (logger->descriptor < 0) {
    free(logger);
    return NULL;
  }
  strftime(timeString, 64, "Init logfile: %c\n", time_info);
  timeStringSize = strlen(timeString);
  if(write(logger->descriptor, timeString, timeStringSize) != timeStringSize) {
    free(logger);
    return NULL;
  }
  return logger;
}

int write_logger(logger_t logger, char* buffer) {
  time_t current_time;
  struct tm *time_info;
  char timeString[16];
  int timeStringSize;
  int ret = 0;

  time(&current_time);
  time_info = localtime(&current_time);

  strftime(timeString, 16, "%H:%M:%S - ", time_info);
  timeStringSize = strlen(timeString);
  if (sem_wait(&logger->semaphore) == -1) return -1;
  if (write(logger->descriptor, timeString, timeStringSize) != timeStringSize) {
    if (sem_post(&logger->semaphore) == -1) return -1;
    return ret;
  }
  ret += write(logger->descriptor, buffer, strlen(buffer));
  if (sem_post(&logger->semaphore) == -1) return -1;
  return ret;
};

void close_logger(logger_t logger) {
  close(logger->descriptor);
  sem_close(&logger->semaphore);
  free(logger);
}
