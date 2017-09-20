#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "logger.h"

logger_t new_logger(char *name, char* folder) {
  int timeStringSize;
  int bytes_write;
  int ret;
  time_t current_time;
  struct tm * time_info;
  char timeString[64];
  struct stat st = {0};
  logger_t logger = malloc(sizeof(logger_c));

  time(&current_time);
  time_info = localtime(&current_time);
  memset(logger->path, 0, PATH_LEN);
  ret = 0;
  if (name == NULL) return NULL;
  if (folder != NULL) {
    strncat(logger->path, folder, PATH_LEN);
    ret += strlen(folder);
    if (stat(logger->path, &st) == -1) {
      if(mkdir(logger->path, 0700) == -1) return NULL;
    }
    strncat(logger->path, "/",PATH_LEN - ret);
    ret++;
  }
  strncat(logger->path, name, PATH_LEN - ret);
  ret += strlen(name);
  strncat(logger->path, " - ",PATH_LEN - ret);
  ret += 3;
  strftime(timeString, 64, "%c", time_info);
  strncat(logger->path, timeString, PATH_LEN - ret);
  logger->descriptor = open(logger->path, O_WRONLY|O_CREAT|O_TRUNC, 0666);
  if (logger->descriptor < 0) {
    free(logger);
    return NULL;
  }
  timeStringSize = strftime(timeString, 64, "Init logfile: %c\n", time_info);
  while (bytes_write < timeStringSize) {
    ret = write(logger->descriptor, timeString + bytes_write, timeStringSize - bytes_write);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) {
      free(logger);
      return NULL;
    }
    bytes_write += ret;
  }
  if (sem_init(&logger->semaphore, 0, 1) == -1) {
    free(logger);
    return NULL;
  }
  return logger;
}

int write_logger(logger_t logger, const char* format, ...) {

  if(logger == NULL) return -1;

  int       ret;
  int       bytes_write = 0;
  int       buffer_len;
  int       timeStringSize;
  char      timeString[16];
  char      buffer[4096];
  va_list   args;
  time_t    current_time;
  struct tm *time_info;

  va_start(args, format);
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);
  buffer_len = strlen(buffer);

  time(&current_time);
  time_info = localtime(&current_time);
  timeStringSize = strftime(timeString, 16, "%H:%M:%S - ", time_info);

  if (sem_wait(&logger->semaphore) == -1) return -1;
  while (bytes_write < timeStringSize) {
    ret = write(logger->descriptor, timeString + bytes_write, timeStringSize - bytes_write);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) return -1;
    bytes_write += ret;
  }
  bytes_write = 0;
  while (bytes_write < buffer_len) {
    ret = write(logger->descriptor, buffer + bytes_write, buffer_len + bytes_write);
    if (ret == -1 && errno == EINTR) continue;
    if (ret == -1) return -1;
    bytes_write += ret;
  }
  if (sem_post(&logger->semaphore) == -1) return -1;
  return bytes_write;
}

void close_logger(logger_t logger) {

  if (logger == NULL) return;

  int ret;
  while (1) {
    ret = close(logger->descriptor);
    if (ret == -1 && errno == EINTR) continue;
    break;
  }
  sem_close(&logger->semaphore);
  free(logger);
}
