#include "logger.h"

logger_t* new_log     (const char* path, int flags, mode_t mode) {
  logger_t* logger = malloc(sizeof(logger_t));
  if ((logger->fd = open(path, flags, mode)) < 0) return NULL;
  if((sem_init(&logger->ls, 0, 1)) < 0) return NULL;
  return logger;
}

int       write_log   (logger_t* logger, const char* str, ...) {
  int ret;
  char buffer[256];
  va_list args;

  va_start (args, str);
  ret = vsprintf (buffer, str, args);
  if (ret < 0) return -1;
  va_end (args);

  if (sem_wait(&logger->ls) < 0){
    fprintf(stderr, "Impossibile scrivere sul file di log\n");
    if(DEBUG) perror("write_log: error in sem_wait");
    return -1;
  }

  ret = write(logger->fd, buffer, strlen(buffer));
  if (ret < 0) return -1;
  if (sem_post(&logger->ls) < 0) {
    fprintf(stderr, "Impossibile scrivere sul file di log\n");
    if(DEBUG) perror("destroy_log: error in sem_wait");
    return -1;
  }
  return ret;
}

int       destroy_log (logger_t* logger) {
  if (close(logger->fd) < 0) {
    fprintf(stderr, "Impossibile terminare il logger correttamente\n");
    if(DEBUG) perror("destroy_log: error in close");
    return 0;
  }
  if (sem_destroy(&logger->ls) < 0) {
    fprintf(stderr, "Impossibile terminare il logger correttamente\n");
    if(DEBUG) perror("destroy_log: error in sem_destroy");
    return -1;
  }
  free(logger);
  return 1;
}
