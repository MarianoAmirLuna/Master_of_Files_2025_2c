#ifndef WORKER_BASE_H
#define WORKER_BASE_H

#include "inc/common.h"
#include "modules/sockets/network.h"
//#include "modules/managers/socket_manager.h"
#include "modules/managers/threads_manager.h"
//#include "modules/managers/pseudocode_manager.h"*/

#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"


config_worker cw;
op_code_module itself_ocm;
int sock_master,sock_storage;


#endif