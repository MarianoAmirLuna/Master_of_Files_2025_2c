#ifndef INICIALIZAR_WORKER_H
#define INICIALIZAR_WORKER_H

#include "semaphore.h"
#ifndef FASE_EXECUTE_H
#include "fase_execute.h"
#endif

void inicializar_semaforos()
{
    sem_init(&sem_query_recibida, 0, 0);
}

void inicializar_variables()
{
    is_free=true;
}

void inicializar_listas(){
    archivos_cargados = list_create();
    lista_tablas_pags = list_create();
}

void inicializar_worker()
{
    inicializar_semaforos();
    inicializar_variables();
    inicializar_listas();
}


    
#endif