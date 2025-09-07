#ifndef BASE_SCHEDULER_H
#define BASE_SCHEDULER_H

#include "libs/config.h"
#include "semaphore.h"
#include "inc/common.h"
#include "modules/sockets/network.h"
#include "inc/libs.h"
#include <pthread.h>
#include "exts/array_ext.h"
#include "exts/list_ext.h"

config_master cm;
/// @brief struct de query
t_list* queries;
/// @brief Es como PID, es incremental desde 0, doc página 13
int query_idx=0;

int increment_idx(){
    
    //TODO: Usar Pthread lock  o semáforo para bloquear el subproceso así no hay condición de carrera al incrementar la query_id
    return query_idx++;
}

#endif