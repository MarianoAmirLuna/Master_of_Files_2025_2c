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
sem_t sem_incoming_client;
//sem_t sem_desalojo;
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
    if(w == NULL){
        log_error(logger, "W es nulo en desalojo (%s:%d)", __func__, __LINE__);
        return 0;
    }
    
    if(w->is_free){
        return 0;
    }
    log_pink(logger, "DESALOJO IS INVOKED");
    t_packet* pdes = create_packet();
    add_int_to_packet(pdes, REQUEST_DESALOJO);
    send_and_free_packet(pdes, w->fd); //Envío y espero su respuesta de success

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
        q->sp = STATE_READY;
    }
    
    query_to(q, q->sp);
    w->is_free = 1; //El worker ahora está libre
    on_changed(on_query_state_changed, q);
    execute_worker(); //Ya que desalojé algo y debería asignarle algún trabajo
    return w->resp_desalojo.status;
}

void* desalojo_worker_query(void* params)
{
    log_orange(logger, "Desalojo worker query is invoked");
    t_list* par = (t_list*)params;
    worker* w = (worker*)list_get(par, 0);
    query* q_worker = (query*)list_get(par, 1);
    query* q_changed = (query*)list_get(par, 2);
    if(w == NULL){
        log_error(logger, "W es nulo en desalojo (%s:%d)", __func__, __LINE__);
        return NULL;
    }
    
    if(w->is_free){
        return NULL;
    }
    log_pink(logger, "DESALOJO IS INVOKED");
    t_packet* pdes = create_packet();
    add_int_to_packet(pdes, REQUEST_DESALOJO);
    send_and_free_packet(pdes, w->fd); //Envío y espero su respuesta de success

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
        q->sp = STATE_READY;
    }
    
    //w->is_free = 1; //El worker ahora está libre
    on_changed(on_query_state_changed, q);
    //execute_worker(); //Ya que desalojé algo y debería asignarle algún trabajo

    w->is_free = 1; //El worker ahora está libre
    w->id_query = -1; //El worker ya no tiene query asignado    
    log_info(logger, "## Se desaloja la Query <%d> (%d) del Worker <%d> - Motivo: PRIORIDAD",
        q_worker->id,
        q_worker->priority,
        w->id
    );
    query_to(q_worker, STATE_READY); //El query que estaba en exec pasa a ready        
    execute_this_query_on_this_worker(q_changed, w);
    list_destroy(par);
    //return w->resp_desalojo.status;
    return NULL;
}


int increment_priority(query* q){
    //Recordar que cuanto menor es el número mayor es su prioridad. No confundir.
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

int increment_idx(){
    //Thread-safe
    sem_wait(&sem_idx);
    int aux = query_idx;
    query_idx++;
    sem_post(&sem_idx);
    return aux;
}

void query_to_no_notify(query* q, state_process to){
    if(to == STATE_EXEC){
        if(q->temp == NULL){
            q->temp = temporal_create();
        }
        else{
            temporal_restart(q->temp);
        }
    }
    if(is_list_sp(q->sp))
    {
        t_list* l= get_list_by_sp(q->sp); 
        list_remove_by_condition_by(l, by_query_qid, (void*)q->id); //remove from this 
    }
    if(is_queue_sp(q->sp)){
        //el que estaba en READY que pasa a EXIT debo removerlo de la pila
        t_queue* l = get_queue_by_sp(q->sp);
        list_remove_by_condition_by(l->elements, by_query_qid, (void*)q->id); //remove from this 
    }
    q->sp = to;
    add_query_on_state(q, to); //No le importa el t_queue por que t_queue se invocó previamente un pop y ese pop ya lo removió
}

void query_to(query* q, state_process to){
    if(q->sp == STATE_EXIT && to == STATE_READY)
    {
        log_error(logger, "is not valid from %s to %s state process returned is invoked", 
            state_to_string(q->sp),
            state_to_string(to)
        );
        return;
    }
    /*if(q->sp != STATE_READY && to != STATE_READY){
        if(!is_valid_sp(q->sp, to)){
            log_error(logger, "is not valid from %s to %s state process returned is invoked", 
                state_to_string(q->sp),
                state_to_string(to)
            );
            return;
        }
    }*/
    if(to == STATE_EXEC){
        if(q->temp == NULL){
            q->temp = temporal_create();
        }
        else{
            temporal_restart(q->temp);
        }
    }
    if(is_list_sp(q->sp))
    {
        t_list* l= get_list_by_sp(q->sp); 
        list_remove_by_condition_by(l, by_query_qid, (void*)q->id); //remove from this 
    }
    if(is_queue_sp(q->sp)){
        //el que estaba en READY que pasa a EXIT debo removerlo de la pila
        t_queue* l = get_queue_by_sp(q->sp);
        list_remove_by_condition_by(l->elements, by_query_qid, (void*)q->id); //remove from this 
    }
    q->sp = to;
    add_query_on_state(q, to); //No le importa el t_queue por que t_queue se invocó previamente un pop y ese pop ya lo removió
    on_changed(on_query_state_changed, q);
}

void print_query(query* q){
    log_orange(logger, "ID: %d, State Process: %s, Priority: %d, PC=%d", q->id, state_to_string(q->sp), q->priority, q->pc);
}
void print_worker(worker* w){
    log_orange(logger, "ID=%d, ID_QUERY=%d, ISFree: %d", w->id, w->id_query, w->is_free);
}

void print_queries(){
    log_light_blue(logger, "Start print queries");
    int sz = list_size(queries);
    for(int i=0;i<sz;i++){
        print_query(cast_query(list_get(queries, i)));
    }
    log_light_blue(logger, "End print queries");
}
    
void print_query_de_la_pila(){
    t_queue* qqq = get_queue_by_sp(STATE_READY);
    
    log_light_blue(logger, "Start print queries DE PILA");
    int sz = list_size(qqq->elements);
    for(int i=0;i<sz;i++){
        print_query(cast_query(list_get(qqq->elements, i)));
    }
    log_light_blue(logger, "End print queries DE PILA");
}

void print_workers(){
    log_light_blue(logger, "Start print workers");
    int sz = list_size(workers);
    for(int i=0;i<sz;i++){
        print_worker(cast_worker(list_get(workers, i)));
    }
    log_light_blue(logger, "End print workers");
}
#endif