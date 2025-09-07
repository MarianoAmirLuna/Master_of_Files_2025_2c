#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif

#include "exts/common_ext.h"
query* get_query_available(){
    //Por prioridad planif, blabla debo buscar el query
    log_warning(logger, "get_query_available NOT IMPLEMENTED");
    return NULL;
}

void execute_worker(){
    log_light_blue(logger, "%s", "On ExecuteWorker");
    
    //TODO: Tengo que agarrar un worker libre (si lo hay) y si existe algún query a ejecutar en ready tengo que agarrar ese y mandarlo a EXEC

    int sz =list_size(workers);
    for(int i=0;i<sz;i++){
        worker* w = list_get(workers, i);
        //Obviamente hace falta mejorar esta poronga
        /*if(w->id_query != -1 && w->sp == STATE_READY){
            //A este worker no se asignó un query y está en ready lo puedo mandar a ejecutar seleccionando la query en base a prioridad blabla
            query* q = get_query_available();
            if(q != NULL){

                t_packet* p =create_packet();
                add_int_to_packet(p, EJECUTAR_QUERY);
                add_string_to_packet(p, q->archive_query); //enviarle el nombre del query a ejecutar
                add_int_to_packet(p, q->pc);
                w->id_query = q->id;
                send_and_free_packet(p, w->fd);
            }
        }*/
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
    }   
}

#endif