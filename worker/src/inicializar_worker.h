#ifndef INICIALIZAR_WORKER_H
#define INICIALIZAR_WORKER_H

#include "semaphore.h"
#ifndef FASE_EXECUTE_H
#include "fase_execute.h"
#endif

void inicializar_semaforos()
{
    sem_init(&sem_need_stop, 0, 0);
    sem_init(&sem_need_desalojo, 0, 0);
    sem_init(&sem_query_recibida, 0, 0);
    sem_init(&tabla_pag_en_uso, 0, 1);
    sem_init(&tabla_frame_en_uso, 0, 1);
    sem_init(&sem_file_tag_buscado, 0, 0);
    sem_init(&sem_bloque_recibido, 0, 0);
    sem_init(&sem_get_data, 0, 0);
    sem_init(&sem_storage_conectado, 0, 0);
    sem_init(&sem_respuesta_storage, 0, 0);
    sem_init(&sem_dimi, 0, 0);
    sem_init(&fin_de_flush, 0, 0);
    sem_init(&sem_de_esperar_la_puta_respuesta, 0, 0);
}

void inicializar_variables()
{
    actual_worker->is_free=true;
    hubo_error=false;
    desalojado_por_prioridad=false;
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
    //inicializar_memoria();
}


    
#endif