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

//OK
void on_query_state_changed(void* elem){
    
    query* q = cast_query(elem);
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
    log_light_blue(logger, "Un query (ID=%d,Archivo=%s) cambió de estado valor: %s", q->id, q->archive_query, state_to_string(q->sp));
    if(q->sp == STATE_EXIT){
        t_packet* p = create_packet();
        add_int_to_packet(p, REQUEST_KILL); //Según el diagrama algo que está en EXIT nunca pasa a otro estado. IS GONE
        add_string_to_packet(p, "Desalojado");
        send_and_free_packet(p, q->fd); //Notifico al Query Control que se desaloja a la mierda
    }
}

void on_query_priority_changed(void* elem){
    
    query* q = cast_query(elem);
    log_light_blue(logger, "Un query cambió de prioridad");
    //if(!list_exists(queries, is_most_priority, q))
    //    return;
    
    //Desalojar un worker que tenga un query de menor prioridad (número mayor)
    for(int i=0;i<list_size(workers);i++){
        worker* w = cast_worker(list_get(workers, i));
        if(w->id_query == -1){
            log_warning(logger, "Oh ");
            continue;
        }

        query* q_worker = get_query_by_qid(w->id_query);
        if(q->id == q_worker->id){
            log_light_green(logger,"Ambos query tienen mismos ID se lo ignora");
            continue;
        }
        log_orange(logger, "Existe worker ID=%d y ID_QUERY=%d",w->id, w->id_query);
        if(w == NULL || q_worker == NULL){
            continue;
        }
        
        if(q_worker->priority <= q->priority)
            continue;
        log_debug(logger, "Voy a desalojarlo");

        desalojo_worker_de_este_query(q_worker);

        break;
    }
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
    //sem_post(&sem_locker);
}


#endif