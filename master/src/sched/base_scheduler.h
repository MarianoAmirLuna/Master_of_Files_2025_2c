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
#include "exts/temporal_ext.h"

pthread_mutex_t mutex_sched;

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
sem_t sem_locker;
sem_t sem_worker;
void execute_worker();

//On predicates
query* get_query_available();
worker* get_worker_by_fd(int fd, int* idx);
int is_valid_sp(state_process from, state_process to);
int is_queue_sp(state_process sp);
int is_list_sp(state_process sp);
t_list* get_list_by_sp(state_process sp);
t_queue* get_queue_by_sp(state_process sp);
void add_query_on_state(query* q, state_process sp);
int count_by_sp(state_process sp);
worker* cast_worker(void* elem);
query* cast_query(void* elem);
int cast_int(void* elem);
bool order_query_by(void* a, void* b);
bool by_worker_free(void* elem);
int by_worker_fd(void* elem, void *by);
int by_worker_wid(void* elem, void *by);
int by_worker_qid(void* elem, void *by);
int by_query_qid(void* elem, void* by);
worker* get_first_worker_free();
worker* get_worker_by_fd(int fd, int* idx);
worker* get_worker_by_qid(qid id);
worker* get_worker_by_wid(wid id);
query* get_query_by_qid(qid id);


void on_query_state_changed(void* elem);
void on_query_priority_changed(void* elem);
void on_changed(void(*cbMeth)(void*), void* argsMeth);

int desalojo(worker* w)
{
    log_pink(logger, "DESALOJO IS INVOKED");
    if(w == NULL){
        log_error(logger, "W es nulo en desalojo (%s:%d)", __func__, __LINE__);
        return;
    }
    
    if(w->is_free){
        return;
    }
    t_packet* pdes = create_packet();
    add_int_to_packet(pdes, REQUEST_DESALOJO);
    send_and_free_packet(pdes, w->fd); //Envío y espero su respuesta de success

    t_list* re = recv_operation_packet(w->fd);
    int resp_success = list_get_int(re,0) == SUCCESS;
    if(!resp_success)
    {
        log_warning(logger, "No se pudo desalojar el worker %d retorno un valor distinto de SUCCESS", w->id);
        return;
    }else{
        log_pink(logger, "Desalojó satisfactoriamente el worker %d", w->id);
    }

    qid qid = list_get_int(re, 1);
    int pc = list_get_int(re, 2);
    query* q = get_query_by_qid(qid);
    if(q != NULL){
        q->pc = pc;
    }
    w->is_free = 1; //El worker ahora está libre
    return resp_success;
}

int increment_priority(query* q){
    //Recordar que cuanto menor es el número mayor es su prioridad. No confundir.
    //Ejemplo, Prioridad =0 es máxima, Prioridad = 4 es baja, etc.
    int old_priority = q->priority;
    q->priority--;
    if(q->priority <= 0)
        q->priority = 0;
    log_info(logger, "## %d Cambio de prioridad: %d - %d", q->id, old_priority, q->priority);
    on_changed(on_query_priority_changed, q);
    return q->priority;
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
        log_error(logger, "is not valid from %s to %s state process returned is invoked", 
            state_to_string(q->sp),
            state_to_string(to)
        );
        return;
        //exit(EXIT_FAILURE);
    }
    if(to == STATE_EXEC){
        q->temp = temporal_create();
        //MANDAR AL WORKER PARA QUE LO EJECUTE
    }
    if(is_list_sp(q->sp))
    {
        t_list* l= get_list_by_sp(q->sp); 
        list_remove_by_condition_by(l, by_query_qid, (void*)q->id); //remove from this 
        add_query_on_state(q, to); //No le importa el t_queue por que t_queue se invocó previamente un pop y ese pop ya lo removió
    }
    q->sp = to;
    on_changed(on_query_state_changed, q);
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