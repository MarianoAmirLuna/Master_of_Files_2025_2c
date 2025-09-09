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

worker* cast_worker(void* elem){
    if(elem == NULL)
        return NULL;
    return (worker*)elem;
}


query* cast_query(void* elem){
    if(elem == NULL)
        return NULL;
    return (query*)elem;
}
int by_worker_free(void* elem, void *by){
    worker* wa = (worker*)elem;
    int byval = (int)by;
    return wa->is_free == byval;
}


int by_worker_fd(void* elem, void *by){
    worker* wa = (worker*)elem;
    int byval = (int)by;
    return wa->fd == byval;
}

int by_worker_wid(void* elem, void *by){
    worker* wa = (worker*)elem;
    wid byval = (wid)by;
    return wa->id == byval;
}

int by_worker_qid(void* elem, void *by){
    worker* wa = (worker*)elem;
    qid byval = (qid)by;
    return wa->id_query == byval;
}

int by_query_qid(void* elem, void* by){
    query* qa = (query*)elem;
    qid byval = (qid)by;
    return qa->id == byval;
}

worker* get_first_worker_free()
{
    void* w = list_find_by(workers, by_worker_free, (int)1);
    if(w == NULL)
        return w;
    return (worker*)w;
}

worker* get_worker_by_fd(int fd, int* idx){
    return cast_worker(list_find_by_idx_list(workers, by_worker_fd, fd, idx));
}

worker* get_worker_by_qid(qid id){
    return cast_worker(list_find_by(workers, by_worker_qid, id));
}

worker* get_worker_by_wid(wid id){
    return cast_worker(list_find_by(workers, by_worker_wid, id));
}

query* get_query_by_qid(qid id){
    return cast_query(list_find_by(queries, by_query_qid, id));
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