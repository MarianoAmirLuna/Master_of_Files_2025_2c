#ifndef SCHEDULER_H
#define SCHEDULER_H

#ifndef BASE_SCHEDULER_H
#include "base_scheduler.h"
#endif

#include "exts/common_ext.h"

//Se debe instanciar en un nuevo subproceso
void scheduler(){

    log_info(logger, "%s", "SCHEDULER INSTANCE");
    if(cm.tiempo_aging < 0){
        log_error(logger, "Tiempo Aging inválido exit(1) as INVOKED");
        //exit(EXIT_FAILURE);
    }
    //Debería usar semáforo acá sólo para habilitar este thread cuando se esté corriendo sino habrá desplazamiento de tiempo aging
    for(;;){
        log_pink(logger, "%s", "ON AGING sleep");
        msleep(cm.tiempo_aging);
        
    }   
}

#endif