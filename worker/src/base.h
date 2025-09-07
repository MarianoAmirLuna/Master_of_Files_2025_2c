#ifndef WORKER_BASE_H
#define WORKER_BASE_H

#include "inc/common.h"
#include "stdio.h"
#include "modules/sockets/network.h"
//#include "modules/managers/socket_manager.h"
#include "modules/managers/threads_manager.h"
//#include "modules/managers/pseudocode_manager.h"
#include "pthread.h"

#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"

//#include "semaphore.h"
#include "funciones_worker.h"
#include "memoria.h"

op_code_module itself_ocm;
int sock_master,sock_storage;
void* memory;
int id_worker;




#endif