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
    query* q = cast_query(elem);
    log_light_blue(logger, "Un query cambió de estado valor: %s", state_to_string(q->sp));
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
    if(q->sp == STATE_EXIT){
        t_packet* p = create_packet();
        add_int_to_packet(p, REQUEST_KILL); //Según el diagrama algo que está en EXIT nunca pasa a otro estado. IS GONE
        //TODO NECESITO EL MOTIVO... HOLY SHIT
        add_string_to_packet(p, "Motivo... OPA necesito el motivo...");
        send_and_free_packet(p, q->fd); //Notifico al Query Control que se
    }
}

void on_query_priority_changed(void* elem){
    query* q = cast_query(elem);
    log_light_blue(logger, "Un query cambió de prioridad");
    //Comprueba si existe algún query que tenga menor prioridad (número mayor) que este.
    if(!list_exists(queries, is_most_priority, q))
        return;
    //Desalojar un worker que tenga un query de menor prioridad (número mayor)
    for(int i=0;i<list_size(workers);i++){
        worker* w = cast_worker(list_get(workers, i));
        log_orange(logger, "Existe worker y su id_query es %d", w->id_query);
        query* q_worker = get_query_by_qid(w->id_query);
        if(w == NULL || q_worker == NULL){
            //Holy shit
            continue;
        }
        if(q_worker->priority <= q->priority)
            continue;
        log_debug(logger, "Voy a desalojarlo");
        t_packet* p = create_packet();
        add_int_to_packet(p, REQUEST_DESALOJO);
        add_int_to_packet(p, q->id);
        send_and_free_packet(p, w->fd); //Envío y espero su respuesta de success

        t_list* re = recv_operation_packet(w->fd);
        if(list_get_int(re,0) != SUCCESS)
        {
            log_error(logger, "No se pudo desalojar el worker %d retorno un valor distinto de SUCCESS", w->id);
            continue; //Debería hacer continue???
        }else{
            log_pink(logger, "Desalojó satisfactoriamente el worker %d", w->id);
        }

        w->is_free = 1; //El worker ahora está libre
        log_info(logger, "## Se desaloja la Query %d (%d) y comienza a ejecutar la Query %d (%d) en el Worker %d",
            q_worker->id,
            q_worker->priority,
            q->id,
            q->priority,
            w->id
        );
        w->id_query = -1; //El worker ya no tiene query asignado    
        
        execute_this_query_on_this_worker(q, w);
        
        query_to(q_worker, STATE_READY); //El query que estaba en exec pasa a ready        
    }
    if(q == NULL){
        log_error(logger, "%s:%d", __func__, __LINE__);
    }
}


#endif