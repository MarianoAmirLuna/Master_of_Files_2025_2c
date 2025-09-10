#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif

#include "exts/common_ext.h"

query* get_query_available(){
    if(cm.algoritmo_planificacion == FIFO){
        t_queue* q = get_queue_by_sp(STATE_READY);
        if(queue_size(q) > 0) //Surprise motherfucker
            return cast_query(queue_pop(q));
        /*int sz = list_size(queries);
        for(int i=0;i<sz;i++){
            query* q = (query*)list_get(queries, i);
            if(q->sp == STATE_READY)
                return q;
        }*/
        return NULL;
    }
    if(cm.algoritmo_planificacion == PRIORITIES){
        log_error(logger, "%s (%s:%d)", "PLANIF PRIORITIES NOT IMPLEMENTED", __func__, __LINE__);
    }
    //Por prioridad planif, blabla debo buscar el query
    log_warning(logger, "get_query_available NOT IMPLEMENTED");
    return NULL;
}

int have_query_ready(){
    return queue_size(get_queue_by_sp(STATE_READY)) > 0;
}


void execute_worker(){
    log_light_blue(logger, "%s", "On ExecuteWorker");
    
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
    t_packet* p = create_packet();
    add_int_to_packet(p, EJECUTAR_QUERY);
    add_string_to_packet(p, q->archive_query); //enviarle el nombre del query a ejecutar
    add_int_to_packet(p, q->pc);
    w->id_query = q->id;
    
    send_and_free_packet(p, w->fd);

    t_list* pack = recv_operation_packet(w->fd); //Debería primero recibir después de esto para saber si fue SUCCESS el ejecutar query?? antes de setear como libre el worker.
    if(list_get_int(pack, 0) == SUCCESS){
        w->is_free=0;
    }
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
        msleep(cm.tiempo_aging);
        //Luego de dormir el subproceso si el algoritmo es de Prioridad debo ir disminuyendo el priority para que este sea más prioritario.
    }   
}

#endif