#ifndef INICIALIZAR_WORKER_H
#define INICIALIZAR_WORKER_H

//#include "base.h"
#include "semaphore.h"

sem_t sem_query_recibida;
/// @brief Cuando el worker está libre para una nueva ejecución que quiera hacer el Master
int is_free;

void inicializar_semaforos()
{
    sem_init(&sem_query_recibida, 0, 0);
}

void inicializar_variables()
{
    is_free=true;
}

void inicializar_worker()
{
    inicializar_semaforos();
    inicializar_variables();
}



#endif