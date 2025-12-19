#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif

#include "exts/common_ext.h"

query* get_query_available(){
    t_queue* q = get_queue_by_sp(STATE_READY);
    if(queue_is_empty(q)){
        log_warning(logger, "Queue READY está empty");
        return NULL;
    }
    if(cm.algoritmo_planificacion == PRIORITIES){
        //Obtener el primer query de más alta prioridad
        list_sort(q->elements, order_query_by); //NOTE: El t_queue pop lo que hace es invocar el list_remove(lista, 0);
    }
    return cast_query(queue_pop(q));
}

int have_query_ready(){
    return queue_size(get_queue_by_sp(STATE_READY)) > 0;
}

void execute_worker_on_this_query(worker* w, query* q){

    if(w->id_query != -1 && !w->is_free){
        _desalojo(w);
        //Tiene query asignado y no está libre. Así que como tengo que ejecutar esto voy a asignarlo
    }

    w->id_query = q->id;
    log_info(logger, "## Se envía la Query %d (%d) al Worker %d", q->id, q->priority, w->id);
    t_packet* p = create_packet();
    add_int_to_packet(p, REQUEST_EXECUTE_QUERY);
    add_int_to_packet(p, q->id); //enviar el id_query
    add_int_to_packet(p, q->pc);
    add_string_to_packet(p,  q->archive_query); //enviarle el nombre del query a ejecutar
    send_and_free_packet(p, w->fd);
    log_pink(logger, "Se manda la query (ID=%d, PC=%d) a ejecutar en Worker=%d", q->id, q->pc, w->id);

    //Le hago saber al query que mandé algo a ejecutar
    t_packet* pq = create_packet();
    add_int_to_packet(pq, REQUEST_EXECUTE_QUERY);
    send_and_free_packet(pq, q->fd);  //Debido al TP imprime en Query Control la solicitud de ejec. Creería que debo notificarlo
    
    query_to(q, STATE_EXEC);
    w->is_free=0; //Ya no está libre porque se asignó
}

void worker_con_prioridad_mas_baja(void* a, void *b){
    worker* wa = cast_worker(a);
    worker* wb = cast_worker(b);
    int qida = wa->id_query;
    int qidb = wb->id_query;
    return get_query_by_qid(qida)->priority >= get_query_by_qid(qidb)->priority ? wa : wb;
}
void query_ready_con_prioridad_mas_alta(void* a, void *b){
    return cast_query(a)->priority <= cast_query(b)->priority;
}

void execute_worker(){
    log_light_blue(logger, "%s", "On ExecuteWorker");
    //print_queries_y_workers();
    if(!have_query_ready()){
        log_pink(logger, "No hay query que se encuentra en ready");
        //No hay query ready así que no hay nada que hacer.
        return;
    }
    if(cm.algoritmo_planificacion == PRIORITIES && cm.tiempo_aging == 0){
        //Si el tiempo aging está desactivado debe verificar si existe un worker que tiene asignado una query con prioridad más baja que el query READY
        
        t_queue* qpr = get_queue_by_sp(STATE_READY);
        list_sort(qpr->elements, order_query_by); 
        query* peek= cast_query(queue_peek(qpr));
        if(peek == NULL){
            log_error(logger, "PEEK QUERY es NULL");
        }
        //Tengo que verificar si existe un query READY que tiene más prioridad que la prioridad más baja que tiene el worker
        worker* wlowpriority = list_get_maximum(workers, worker_con_prioridad_mas_baja);
        if(wlowpriority == NULL){
            log_error(logger, "WLOWPRIORITY es NULL");
        }
        if(wlowpriority->id_query == -1 || wlowpriority->is_free)
        {
            //No tiene nada asignado así que voy a asignar esta query
            execute_worker_on_this_query(wlowpriority, queue_pop(qpr));
            return;
        }
        if(get_query_by_qid(wlowpriority->id_query)->priority >= peek->priority)
        {
            //Existe un query que está en wlowpriority que tiene prioridad más baja (número más alto) que el query que está en READY (de la prioridad más alta)
            //así que debo desalojarlo y ejecutar en este wlowpriority la query que más prioridad tiene (número bajo)
            _desalojo(wlowpriority);
            execute_worker_on_this_query(wlowpriority, queue_pop(qpr));
            return;
        }else{
            log_pink(logger, "HERE ELSE");
            return;
        }
        /*queue_destroy(qpr);
        list_destroy()*/
    }
    worker* w = get_first_worker_free();
    if(w == NULL)
    {
        log_pink(logger, "No hay worker para que ejecute");
        return;
    }
    //Necesito comprobar si hay worker libre antes de hacer pop al queue ready sino se pone fea la cosa.
    query* q= get_query_available();
    if(q == NULL){
        log_warning(logger, "No hay queries en READY");
        return;
    }
    execute_worker_on_this_query(w,q);
    /*pthread_t* pth = malloc(sizeof(pthread_t));
    t_list* params = list_create();
    list_add(params, w);
    list_add(params, q);
    
    //execute_this_query_on_this_worker(q,w);
    pthread_create(pth, NULL, execute_this_query_on_this_worker_v2_thread, (void*)params);
    pthread_detach(*pth);*/
}


void* scheduler(void* params){

    if(cm.tiempo_aging < 0){
        log_error(logger, "Tiempo Aging inválido exit(1) as INVOKED");
        exit(EXIT_FAILURE);
    }
    sem_wait(&sem_have_worker);
    sem_wait(&sem_have_query);
    
    for(;;){
        execute_worker();
        int ts = cm.tiempo_aging <= 0 ? 250 : cm.tiempo_aging;
        msleep(ts); //Divido por 4 para prevenir posible margen de error en temporal_gettime tiempo agging
        
        if(cm.algoritmo_planificacion == FIFO){
            continue;
        }
        if(cm.tiempo_aging <= 0) //al ser sin aging no debe incrementar prioridad no es relevante, por lo que tiene en cuenta el número de prioridad y prioriza la misma.
            continue;
        int sz = list_size(queries);
        for(int i=0;i<sz;i++){
            query* q = cast_query(list_get(queries, i));
            if(q == NULL)
                continue; 
            if(q->sp != STATE_READY) //Incrementa prioridad SOLO LOS QUE ESTAN EN READY
                continue;
            
            if(!temporal_is_empty(q->temp) && temporal_gettime(q->temp) > cm.tiempo_aging){
                increment_priority(q);
            }
        }
    }   
}

#endif