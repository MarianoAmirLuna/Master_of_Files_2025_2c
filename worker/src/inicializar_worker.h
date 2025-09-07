#ifndef INICIALIZAR_WORKER_H
#define INICIALIZAR_WORKER_H

#include "base.h"

void inicializar_worker()
{
    inicializar_semaforos();
    inicializar_variables();
}

void inicializar_semaforos()
{
    sem_init(sem_query_recibida, 0, 0);
}

void inicializar_variables()
{
    is_free=true;
}



#endif