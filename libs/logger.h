#include <stdio.h>
#include <fnctl.h>
#include <error.h>
#include <stdlib.h>
#include <unistd.h>
#include <semaphore.h>

typedef struct logger_s {
  int   file_desc;
  sem_t semaphore;
  char* file_name;
} logger_t;

logget_t* new_logger (char* file_name);
int       write_log (logger_t* logger,char* string, void parameters);
int       destroy_log (logger_t logger);
