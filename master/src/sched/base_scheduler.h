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
//TODO: t_queue* of this queries
/// @brief struct de query
t_list* queries;
t_list* workers;

//<state_process, List<query*>>
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

int is_queue_sp(state_process sp){
    return sp == STATE_READY;
}
int is_list_sp(state_process sp){
    return !is_queue_sp(sp);
}

t_list* get_list_by_sp(state_process sp){
    if(sp == STATE_READY){
        log_error(logger, "State Process inválido para retorno de t_list: %s", state_to_string(sp));
        return NULL;
    }
    return (t_list*)dictionary_get(dict_state, state_to_string(sp));
}
t_queue* get_queue_by_sp(state_process sp){
    if(sp != STATE_READY){
        log_error(logger, "State Process inválido para retorno de t_queue: %s", state_to_string(sp));
        return NULL;
    }
    return (t_queue*)dictionary_get(dict_state, state_to_string(sp));
    // debido al FIFO
}

void add_query_on_state(query* q, state_process sp){
    if(is_queue_sp(sp)){
        queue_push(get_queue_by_sp(sp), q);
        return;
    }
    list_add(get_list_by_sp(sp), q);
}

int count_by_sp(state_process sp){
    //void* elem = dictionary_get(dict_state,sp);
    if(is_queue_sp(sp))
        return queue_size(get_queue_by_sp(sp));
    return list_size(get_list_by_sp(sp));
}

worker* cast_worker(void* elem){
    if(elem == NULL){
        log_warning(logger, "elem is null in (%s:%d)", __func__, __LINE__);
        return NULL;
    }
    return (worker*)elem;
}

query* cast_query(void* elem){
    if(elem == NULL){
        log_warning(logger, "elem is null in (%s:%d)", __func__, __LINE__);
        return NULL;
    }
    return (query*)elem;
}
int cast_int(void* elem){
    return (int)elem;
    //TODO: check this
    /*int res = 0;
    memcpy(&res, elem, sizeof(int));
    return res;*/
}

int by_worker_free(void* elem, void *by){
    return ((worker*)elem)->is_free ==cast_int(by);
}


int by_worker_fd(void* elem, void *by){
    return ((worker*)elem)->fd == cast_int(by);
}

int by_worker_wid(void* elem, void *by){
    return ((worker*)elem)->id == cast_int(by);
}

int by_worker_qid(void* elem, void *by){
    return ((worker*)elem)->id_query == cast_int(by);
}

int by_query_qid(void* elem, void* by){
    return ((query*)elem)->id == cast_int(by);
}

worker* get_first_worker_free()
{
    void* w = list_find_by(workers, by_worker_free, (int)1);
    return cast_worker(w);
}

worker* get_worker_by_fd(int fd, int* idx){
    return cast_worker(list_find_by_idx_list(workers, by_worker_fd, fd, idx));
}

worker* get_worker_by_qid(qid id){
    return cast_worker(list_find_by(workers, by_worker_qid, (int)id));
}

worker* get_worker_by_wid(wid id){
    return cast_worker(list_find_by(workers, by_worker_wid, (int)id));
}

query* get_query_by_qid(qid id){
    return cast_query(list_find_by(queries, by_query_qid, (int)id));
}

int increment_idx(){
    //Thread-safe
    sem_wait(&sem_idx);
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

void query_to(query* q, state_process to){
    
    if(!is_valid_sp(q->sp, to)){
        log_error(logger, "is nota valid from to state process exit(1) is INVOKED");
        exit(EXIT_FAILURE);
    }
    if(is_list_sp(q->sp))
    {
        t_list* l= get_list_by_sp(q->sp); 
        list_remove_by_condition_by(l, by_query_qid, (void*)q->id); //remove from this 
        add_query_on_state(q, to); //No le importa el t_queue por que t_queue se invocó previamente un pop y ese pop ya lo removió
    }
    q->sp = to;
}

void print_query(query* q){
    log_orange(logger, "ID: %d, State Process: %s, Priority: %d", q->id, state_to_string(q->sp), q->priority);
}

void print_queries(){
    log_light_blue(logger, "Start print queries");
    int sz = list_size(queries);
    for(int i=0;i<sz;i++){
        print_query(cast_query(list_get(queries, i)));
    }
    log_light_blue(logger, "End print queries");
}
    
#endif