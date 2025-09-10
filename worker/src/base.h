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

op_code_module itself_ocm;
int sock_master,sock_storage;
void* memory;
int id_worker;
config_worker cw;
int block_size;

int storage_block_size;

//mutex por ser globales en los get y add
t_list* archivos_cargados;
t_list* lista_frames;
t_list* lista_tablas_pags;

sem_t sem_query_recibida;
/// @brief Cuando el worker está libre para una nueva ejecución que quiera hacer el Master
int is_free;

int pc_actual;

char* archivo_query_actual;


#endif