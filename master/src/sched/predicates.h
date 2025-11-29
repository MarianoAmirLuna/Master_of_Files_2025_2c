#ifndef SCHED_PREDICATES_H
#define SCHED_PREDICATES_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif


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
    //WARNING: check this
    /*int res = 0;
    memcpy(&res, elem, sizeof(int));
    return res;*/
}

bool order_query_by(void* a, void* b){
    return cast_query(a)->priority < cast_query(b)->priority;
}

bool by_worker_free(void* elem){
    if(elem == NULL)
        return 0;
    return cast_worker(elem)->is_free;
}


int by_worker_fd(void* elem, void *by){
    return cast_worker(elem)->fd == cast_int(by);
}

int by_worker_wid(void* elem, void *by){
    return cast_worker(elem)->id == cast_int(by);
}

int by_worker_qid(void* elem, void *by){
    return cast_worker(elem)->id_query == cast_int(by);
}

int by_query_qid(void* elem, void* by){
    return cast_query(elem)->id == cast_int(by);
}

int by_equal_query_qid_on_worker(void*elem, void* by){
    return cast_worker(elem)->id_query == cast_int(by);
}

worker* get_first_worker_free()
{
    if(list_is_empty(workers))
        return NULL;
        
    void* w = list_find(workers, by_worker_free);
    if(w == NULL){
        log_warning(logger, "No hay workers libres");
        return NULL;
        //log_error(logger, "Ohhh es nulo el elemento wtf");
    }
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

int is_most_priority(void* a, void* b){
    return cast_query(a)->priority < cast_query(b)->priority;
}

/// @brief Comprueba si existe algún workers que tiene asignado esta qid
/// @param id id del query
/// @return bool
int is_query_assigned(qid id){
    return list_exists(workers, by_equal_query_qid_on_worker, (void*)id);
}
/*int by_worker_free(void* elem, void *by){
    worker* wa = (worker*)elem;
    int byval = (int)by;
    if(wa->is_free == byval)
        return 1;
    return 0;
}*/

#endif

