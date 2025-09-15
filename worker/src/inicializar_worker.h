#ifndef INICIALIZAR_WORKER_H
#define INICIALIZAR_WORKER_H

#include "semaphore.h"
#ifndef FASE_EXECUTE_H
#include "fase_execute.h"
#endif

void inicializar_semaforos()
{
    sem_init(&sem_query_recibida, 0, 0);
    sem_init(&tabla_pag_en_uso, 0, 0);
    sem_init(&tabla_frame_en_uso, 0, 0);
}

void inicializar_variables()
{
    is_free=true;
}

void inicializar_colecciones(){
    archivos_cargados = list_create();
    tabla_pags_global = queue_create();
}

void inicializar_worker()
{
    inicializar_semaforos();
    inicializar_variables();
    inicializar_colecciones();
    inicializar_memoria();
}


    
#endif