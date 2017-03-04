#include "logger.h"

logger_t* new_log (const char* path) {
  logger_t* logger = malloc(sizeof(logger_t));

  strcpy(logger->path, path);
  logger->file_desc = fopen(logger->path, "w+");
  sem_init(&logger->log_semaphore, 0, 1);
  fclose(logger->file_desc);
  return logger;
}

int   write_log(logger_t* logger, const char* str, ...) {
  int ret;
  va_list args;

  if (sem_wait(&logger->log_semaphore) < 0){
    fprintf(stderr, "Impossibile scrivere sul file di log\n");
    if(DEBUG) perror("write_log: error in sem_wait");
    return 0;
  }

  FILE* file_desc = fopen(logger->path, "a+");
  va_start (args, str);
  ret = vfprintf (file_desc, str, args);
  va_end (args);
  fclose(logger->file_desc);

  if (sem_post(&logger->log_semaphore) < 0) {
    fprintf(stderr, "Impossibile scrivere sul file di log\n");
    if(DEBUG) perror("destroy_log: error in sem_wait");
    return 0;
  }
  return ret;
}

int       destroy_log (logger_t* logger) {
  fclose(logger->file_desc);
  sem_destroy(&logger->log_semaphore);
  free(logger);
  return 1;
}
