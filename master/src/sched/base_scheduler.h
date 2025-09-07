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
t_list* workers;
/// @brief Es como PID, es incremental desde 0, doc página 13
int query_idx=0;
int degree_multiprocess;
sem_t sem_idx;
void execute_worker();
query* get_query_available();

int increment_idx(){
    sem_wait(&sem_idx);
    //TODO: Usar Pthread lock  o semáforo para bloquear el subproceso así no hay condición de carrera al incrementar la query_id
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

#endif