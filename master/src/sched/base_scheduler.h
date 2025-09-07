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
t_dictionary* dict_state;

/// @brief Es como PID, es incremental desde 0, doc página 13
qid query_idx=0;
int degree_multiprocess;
sem_t sem_idx;
void execute_worker();
query* get_query_available();
worker* get_worker_by_fd(int fd, int* idx);
t_list* get_list_by_sp(state_process sp){
    return (t_list*)dictionary_get(dict_state, state_to_string(sp));
}

worker* get_worker_by_fd(int fd, int* idx)
{
    int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        worker* w = list_get(workers, i);
        if(w->fd == fd){
            *idx = i;
            return w;
        }
    }
    *idx = -1;
    return NULL;
}

worker* get_worker_by_wid(wid id){
    int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        worker* w = list_get(workers, i);
        if(w == NULL){
            continue;
        }
        if(w->id == id)
            return w;
    }
    return NULL;
}

query* get_query_by_qid(qid id){
    int sz = list_size(queries);
    for(int i=0;i<sz;i++){
        query* q = list_get(queries, i);
        if(q == NULL)
            continue;
        if(q->id == id)
            return q;
    }
    return NULL;
}

int increment_idx(){
    sem_wait(&sem_idx);
    //TODO: Usar Pthread lock  o semáforo para bloquear el subproceso así no hay condición de carrera al incrementar la query_id
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

#endif