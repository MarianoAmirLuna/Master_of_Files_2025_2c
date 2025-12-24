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
pthread_mutex_t safe_query_to;
//response_desalojo resp_desalojo= {-1, -1, -1};

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
sem_t sem_incoming_client;
sem_t sem_have_query;
sem_t sem_have_worker;

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

int increment_idx(){
    //Thread-safe
    sem_wait(&sem_idx);
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

int increment_priority(query* q){
    //Recordar que cuanto menor es el número, mayor es su prioridad. No confundir.
    //Ejemplo, Prioridad =0 es máxima, Prioridad = 4 es baja, etc.
    int old_priority = q->priority;
    q->priority--;
    if(old_priority == 0 && q->priority==0)
        return q->priority; //No hubo cambio, es lo mismo
    if(q->priority <= 0)
        q->priority = 0;
    if(old_priority == 0 && q->priority==0)
        return q->priority; //No hubo cambio, es lo mismo
    log_info(logger, "## %d Cambio de prioridad: %d - %d", q->id, old_priority, q->priority);
    on_changed(on_query_priority_changed, q);
    return q->priority;
}

int _desalojo_abstract(worker* w, state_process spto){
    if(w->estoy_desalojando)
        return 1;
    if(w == NULL){
        log_error(logger, "W es nulo en desalojo (%s:%d)", __func__, __LINE__);
        return 1;
    }
    if(w->is_free){
        return 1;
    }
    w->estoy_desalojando= 1;
    log_pink(logger, "DESALOJO IS INVOKED");
    t_packet* pdes = create_packet();
    add_int_to_packet(pdes, REQUEST_DESALOJO_AGING);
    send_and_free_packet(pdes, w->fd); //Envío y espero su respuesta de success
    
    query* q_worker = get_query_by_qid(w->id_query);
    log_light_blue(logger, "Esperando respuesta de desalojo del worker %d", w->id);
    sem_wait(&w->sem_desalojo); //MMM me da medio miedo porque como tiene que flushear la tabla el worker, debe esperar hasta que termine de flushearla el pete.
    log_light_blue(logger, "Termine de esperar respuesta de desalojo del worker %d", w->id);
    if(w->resp_desalojo.status != SUCCESS){
        log_warning(logger, "No se pudo desalojar el worker %d retorno un valor distinto de SUCCESS", w->id);
        return w->resp_desalojo.status;
    }
    else{
        log_pink(logger, "Desalojó satisfactoriamente el worker %d", w->id);
    }

    qid qid = w->resp_desalojo.id_query;
    int pc = w->resp_desalojo.pc;
    log_pink(logger, "Respuesta desalojo: %d, %d, %d", w->resp_desalojo.status, w->resp_desalojo.id_query, w->resp_desalojo.pc);
    query* q = get_query_by_qid(qid);
    if(q != NULL){
        q->pc = pc;
        q->sp = spto;
    }else{
        log_error(logger, "Query es nulo en desalojo (%s:%d)", __func__, __LINE__);
        return 0;
    }
     if(spto != STATE_EXIT && cm.algoritmo_planificacion == PRIORITIES){
        log_info(logger, "## Se desaloja la Query <%d> (%d) del Worker <%d> - Motivo: PRIORIDAD",
            q_worker->id,
            q_worker->priority,
            w->id
        );
    }
    query_to(q, q->sp);
    w->is_free = 1; //El worker ahora está libre
    w->id_query = -1; //El worker ya no tiene query asignado    
    
    return w->resp_desalojo.status;
}

int _desalojo_matar(worker *w){
    return _desalojo_abstract(w, STATE_EXIT);
}

int _desalojo(worker* w)
{
    return _desalojo_abstract(w, STATE_READY);
}

int desalojo_worker_de_este_query(query* q){
    int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        worker* w = cast_worker(list_get(workers, i));
        if(w == NULL){
            log_error(logger, "W es NULL en (%s:%d)", __func__, __LINE__);
            continue;
        }
        if(w->id_query == q->id){
            if(w->estoy_desalojando) //no hago nada porque estoy pidiendo desalojar
                return;
            pthread_t* pth = malloc(sizeof(pthread_t));
            pthread_create(pth, NULL, _desalojo, w);
            pthread_detach(*pth);
            //return _desalojo(w);
        }
    }
    return 0;
}


//PRIVATE FUNC
void _query_to(query* q, state_process to){
    if(q->sp == STATE_EXIT && to == STATE_READY)
    {
        log_error(logger, "is not valid from %s to %s state process returned is invoked", 
            state_to_string(q->sp),
            state_to_string(to)
        );
        return;
    }
    if(to == STATE_EXEC){
        if(q->temp == NULL){
            q->temp = temporal_create();
        }
        else{
            temporal_restart(q->temp);
        }
    }

    t_list* l = is_list_sp(q->sp) ? get_list_by_sp(q->sp) : get_queue_by_sp(q->sp)->elements;
    list_remove_by_condition_by(l, by_query_qid, (void*)q->id); //remove from this 
    
    q->sp = to;
    add_query_on_state(q, to);
}
void query_to(query* q, state_process to){
    pthread_mutex_lock(&safe_query_to);
    _query_to(q, to);
    on_changed(on_query_state_changed, q);
    pthread_mutex_unlock(&safe_query_to);
}

void query_to_no_notify(query* q, state_process to){
    pthread_mutex_lock(&safe_query_to);
    _query_to(q, to);
    pthread_mutex_unlock(&safe_query_to);
}

void print_query(query* q){
    log_orange(logger, "ID: %d, State Process: %s, Priority: %d, PC=%d", q->id, state_to_string(q->sp), q->priority, q->pc);
}
void print_worker(worker* w){
    log_orange(logger, "ID=%d, ID_QUERY=%d, ISFree: %d", w->id, w->id_query, w->is_free);
}

void _print_queries(){
    log_light_blue(logger, "Start print queries");
    int sz = list_size(queries);
    for(int i=0;i<sz;i++){
        print_query(cast_query(list_get(queries, i)));
    }
    log_light_blue(logger, "End print queries");
}
    
void _print_query_de_la_pila(){
    t_queue* qqq = get_queue_by_sp(STATE_READY);
    
    log_light_blue(logger, "Start print queries DE PILA");
    int sz = list_size(qqq->elements);
    for(int i=0;i<sz;i++){
        print_query(cast_query(list_get(qqq->elements, i)));
    }
    log_light_blue(logger, "End print queries DE PILA");
}

void _print_workers(){
    log_violet(logger, "Start print workers");
    int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        print_worker(cast_worker(list_get(workers, i)));
    }
    log_violet(logger, "End print workers");
}

void print_queries_y_workers(){
    _print_queries();
    _print_query_de_la_pila();
    _print_workers();
}
#endif