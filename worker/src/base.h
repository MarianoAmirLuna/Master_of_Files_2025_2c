#ifndef WORKER_BASE_H
#define WORKER_BASE_H

#include "inc/common.h"
#include "stdio.h"
#include "semaphore.h"
#include "modules/sockets/network.h"
//#include "modules/managers/socket_manager.h"
#include "modules/managers/threads_manager.h"
#include "pthread.h"

#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"
#include "commons/collections/queue.h"
#include "libs/instances_struct.h"

bool esta_libre(void *elem);
worker* actual_worker;
query* actual_query;
op_code_module itself_ocm;
int sock_master,sock_storage;
void* memory;
int id_worker;
config_worker cw;
int block_size;
int need_stop;
int storage_block_size;

//mutex por ser globales en los get y add
t_list* lista_frames; //Se conoce el inicio de un frame, no el fin y si esta libre o no
t_list* archivos_cargados;
t_queue* tabla_pags_global;

sem_t sem_need_stop;
sem_t sem_query_recibida;
sem_t tabla_pag_en_uso;
sem_t tabla_frame_en_uso;
sem_t sem_file_tag_buscado;
sem_t sem_bloque_recibido;
sem_t sem_get_data;

/*/// @brief Cuando el worker está libre para una nueva ejecución que quiera hacer el Master
int is_free;*/

char* data_bloque;

char* archivo_query_actual;
char *file_tag_buscado;

marco* frame_buscado;

sem_t* sem_block_size_recibido;

#endif