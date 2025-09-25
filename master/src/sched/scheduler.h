#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif

#include "exts/common_ext.h"

query* get_query_available(){
    t_queue* q = get_queue_by_sp(STATE_READY);
    if(queue_is_empty(q))
        return NULL;
    if(cm.algoritmo_planificacion == PRIORITIES){
        //Obtener el primer query de más alta prioridad
        list_sort(q->elements, order_query_by); //NOTE: El t_queue pop lo que hace es invocar el list_remove(lista, 0);
    }
    return cast_query(queue_pop(q));
}

int have_query_ready(){
    return queue_size(get_queue_by_sp(STATE_READY)) > 0;
}
void execute_this_query_on_this_worker(query* q, worker* w){
    //TODO: Bloquear este subproceso para prevenir condiciones de carrera
    if(q == NULL || w == NULL){
        log_error(logger, "THE FUCK (%s:%d)",__func__,__LINE__);
        return;
    }

    t_packet* p = create_packet();
    add_int_to_packet(p, EJECUTAR_QUERY);
    add_string_to_packet(p, q->archive_query); //enviarle el nombre del query a ejecutar
    add_int_to_packet(p, q->pc);
    w->id_query = q->id;
    log_info(logger, "## Se envía la Query %d al Worker %d", q->id, w->id_query);
    send_and_free_packet(p, w->fd);

    t_packet* pq = create_packet();
    add_int_to_packet(pq, REQUEST_EXECUTE_QUERY);
    send_and_free_packet(pq, q->fd);  //Debido al TP imprime en Query Control la solicitud de ejec. Creería que debo notificarlo
    
    t_list* pack = recv_operation_packet(w->fd); //Debería primero recibir después de esto para saber si fue SUCCESS el ejecutar query?? antes de setear como libre el worker.
    if(list_get_int(pack, 0) == SUCCESS){
        query_to(q, STATE_EXEC);
        w->is_free=0;
    }
}
void execute_worker(){
    log_light_blue(logger, "%s", "On ExecuteWorker");
    
    //TODO: Debo comprobar entre todas las queries tanto EXEC como en READY si existe alguno más prioritario que los que ya se ejecutan en worker para que este la desaloje
    

    //TODO: Tengo que agarrar un worker libre (si lo hay) y si existe algún query a ejecutar en ready tengo que agarrar ese y mandarlo a EXEC
    worker* w = get_first_worker_free();
    if(w == NULL || !have_query_ready()) //De Morgan papá. Viste que es útil la matemática discreta.
        return;

    //Necesito comprobar si hay worker libre antes de hacer pop al queue ready sino se  pone fea la cosa.
    query* q= get_query_available();
    if(q == NULL){
        log_warning(logger, "WTF (%s:%d) exit(1) IS INVOKED",__func__,__LINE__);
        exit(EXIT_FAILURE);
    }

    execute_this_query_on_this_worker(q, w);
}

//Se debe instanciar en un nuevo subproceso
//Coloqué el argumento params para que el compilador no llore con el warning
void* scheduler(void* params){

    //TODO: Se debe asignar un query a un worker libre en base a la prioridad, aging, planificación blablablab, al pedir que se ejecute se debe pasar el path que pide el subpunto 2 del check 1.
    log_info(logger, "%s", "SCHEDULER INSTANCE");
    if(cm.tiempo_aging < 0){
        log_error(logger, "Tiempo Aging inválido exit(1) as INVOKED");
        //exit(EXIT_FAILURE);
    }
    //Debería usar semáforo acá sólo para habilitar este thread cuando se esté corriendo sino habrá desplazamiento de tiempo aging?
    for(;;){
        execute_worker();
        
        log_pink(logger, "%s", "ON AGING sleep");

        int ts = cm.tiempo_aging <= 0 ? 250 : cm.tiempo_aging/4;
        msleep(ts); //Divido por 4 para prevenir posible margen de error en temporal_gettime tiempo agging
        if(cm.tiempo_aging <= 0)
            continue;
        int sz = list_size(queries);
        for(int i=0;i<sz;i++){
            query* q = cast_query(list_get(queries, i));
            if(q == NULL)
                continue; 
            if(q->sp != STATE_READY) //Increment priority only query that is STATE_READY
                continue;
            if(!temporal_is_empty(q->temp)){
                if(temporal_gettime(q->temp) > cm.tiempo_aging)
                    increment_priority(q);
                //TODO: ME faltan los desalojos cuando hay alguno más prioritario que el que la ejecutó la tiene que desalojar
            }
        }
        //Luego de dormir el subproceso si el algoritmo es de Prioridad debo ir disminuyendo el priority para que este sea más prioritario.
    }   
}

#endif