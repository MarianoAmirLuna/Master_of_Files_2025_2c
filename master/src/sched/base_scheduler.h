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
#include "commons/collections/queue.h"

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

int is_valid_sp(state_process from, state_process to){
    if(from ==STATE_READY && to != STATE_READY)
        return 1;
    if(from == STATE_EXEC && (to == STATE_READY || to == STATE_EXIT))
        return 1;
        
    return 0;
}


t_list* get_list_by_sp(state_process sp){
    return (t_list*)dictionary_get(dict_state, state_to_string(sp));
}
t_queue* get_queue_by_sp(state_process sp){
    // debido al FIFO
}

void query_to(query* q, state_process to){
    //if(is_valid_sp (q->))
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
worker* get_worker_by_qid(qid id){
     int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        worker* w = list_get(workers, i);
        if(w == NULL){
            continue;
        }
        if(w->id_query == id)
            return w;
    }
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
    //Thread-safe
    sem_wait(&sem_idx);
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

#endif