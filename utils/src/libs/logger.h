#ifndef LIBS_LOGGER_H
#define LIBS_LOGGER_H
#include "../inc/common.h"

#include "commons/string.h"
#include "commons/log.h"

#include <string.h>

t_log* logger = NULL;

#define DELETE_LOG_EVERY_RUN

/// @brief Crea el loc de variable global
/// @param name Nombre del Log
/// @param level Nivel
/// @return variable global logger
t_log* create_log(char* name, t_log_level level){
    char* log = ".log";
    char* result = malloc(strlen(name)+1);
    strcpy(result, name);
    if(!string_ends_with(name, log))
        string_append(&result, log);
    #ifdef DELETE_LOG_EVERY_RUN
    char* s = string_from_format("%s", result);
    remove(s);
    free(s);
    #endif
    logger = log_create(result, name, 1, level);
    
    #ifdef DELETE_LOG_EVERY_RUN
    log_warning(logger, "%s","RECORDAR QUE EL MACRO 'DELETE_LOG_EVERY_RUN' ESTA DEFINIDO");
    #endif
    
    free(result);
    return logger;
}

void deprecated_log(const char* funcname, int line){
    log_error(logger, "[OBSOLETO] En %s:%d", funcname, line);
}
#endif