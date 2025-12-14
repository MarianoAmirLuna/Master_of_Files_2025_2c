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
/*
void execute_this_query_on_this_worker(query* q, worker* w){
    //TODO: Bloquear este subproceso para prevenir condiciones de carrera
    if(q == NULL || w == NULL){
        log_error(logger, "THE FUCK (%s:%d)",__func__,__LINE__);
        return;
    }
    if(!w->is_free){
        //pthread_t* pth = malloc(sizeof(pthread_t));
        //pthread_create(pth, NULL, desalojo, w);
        //pthread_detach(*pth);
        //desalojo(w);
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
    w->is_free=0;
}


void* execute_this_query_on_this_worker_v2_thread(void* elem){
    t_list* params = (t_list*)elem;
    worker* w = cast_worker(list_get(params,0));
    query* q = cast_query(list_get(params,1));
    //TODO: Bloquear este subproceso para prevenir condiciones de carrera
    if(q == NULL || w == NULL){
        log_error(logger, "THE FUCK (%s:%d)",__func__,__LINE__);
        return;
    }
    //sem_wait(&sem_locker);
    if(!w->is_free){
        //pthread_t* pth = malloc(sizeof(pthread_t));
        //pthread_create(pth, NULL, desalojo, w);
        //pthread_detach(*pth);
        //desalojo(w);
    }
    w->id_query = q->id;
    log_info(logger, "## Se envía la Query %d (%d) al Worker %d", q->id, q->priority,w->id);
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
    w->is_free=0;
    print_queries();
    print_workers();
    list_destroy(params);
    pthread_mutex_unlock(&mutex_sched);
}
*/
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
void execute_worker(){
    log_light_blue(logger, "%s", "On ExecuteWorker");
    print_queries_y_workers();
    worker* w = get_first_worker_free();
    if(!have_query_ready()){
        log_pink(logger, "No hay query que se encuentra en ready");
        return;
    }
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