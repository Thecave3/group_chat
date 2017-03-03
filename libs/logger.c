##include "logger.h"

logget_t* new_logger (char* file_name);
int       write_log (logger_t* logger,char* string, void parameters);
int       destroy_log (logger_t logger);
