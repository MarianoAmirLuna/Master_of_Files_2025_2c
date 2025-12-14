#ifndef SCHED_HANDLERS_H
#define SCHED_HANDLERS_H

#ifndef SCHED_PREDICATES_H
#include "predicates.h"
#endif


void on_changed(void(*cbMeth)(void*), void* argsMeth){
    if(cbMeth != NULL){
        cbMeth(argsMeth);
    }
}

void on_query_state_changed(void* elem){
    /*log_orange(logger, "On query statechanged wait");
    sem_wait(&sem_locker);
    log_orange(logger, "On query statechanged nowwat");*/
    query* q = cast_query(elem);
    log_light_blue(logger, "Un query (ID=%d,Archivo=%s) cambió de estado valor: %s", q->id, q->archive_query, state_to_string(q->sp));
    //log_light_blue(logger, "Un query %d cambió  de estado valor: %s", q->id, state_to_string(q->sp));
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
    if(q->sp == STATE_EXIT){
        t_packet* p = create_packet();
        add_int_to_packet(p, REQUEST_KILL); //Según el diagrama algo que está en EXIT nunca pasa a otro estado. IS GONE
        add_string_to_packet(p, "Desalojado");
        send_and_free_packet(p, q->fd); //Notifico al Query Control que se
    }
    //sem_post(&sem_locker);
}

void on_query_priority_changed(void* elem){
    
    query* q = cast_query(elem);
    log_light_blue(logger, "Un query cambió de prioridad");
    //Comprueba si existe algún query que tenga menor prioridad (número mayor) que este.

    //DANGER: CHECK THIS
//    if(!list_exists(queries, is_most_priority, q))
//        return;
    //sem_wait(&sem_locker);
    //Desalojar un worker que tenga un query de menor prioridad (número mayor)
    //list_distinct(workers, )
    for(int i=0;i<list_size(workers);i++){
        worker* w = cast_worker(list_get(workers, i));
        
        query* q_worker = get_query_by_qid(w->id_query);
        if(q->id == q_worker->id){
            log_light_green(logger,"Ambos query tienen mismos ID se lo ignora");
            continue;
        }
        log_orange(logger, "Existe worker ID=%d y ID_QUERY=%d",w->id, w->id_query);
        if(w == NULL || q_worker == NULL){
            //Holy shit
            continue;
        }
        
        if(q_worker->priority <= q->priority)
            continue;
        log_debug(logger, "Voy a desalojarlo");
        pthread_t* pth = malloc(sizeof(pthread_t));
        t_list* params = list_create();
        list_add(params, w);
        list_add(params, q_worker);
        list_add(params, q);
        /*pthread_create(pth, NULL, desalojo_worker_query, params);
        pthread_detach(*pth);*/
        desalojo(w);
    
        
        break;
    }
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
    //sem_post(&sem_locker);
}


#endif