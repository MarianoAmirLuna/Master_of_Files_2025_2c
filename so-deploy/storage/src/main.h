#include "base.h"
#include "signal.h"

/*#ifndef IO_EXT_H
#include "exts/io_ext.h"
#endif */
//#include "funciones_generales.h"
#include "inicializacion_storage.h"
#include "control_accesos.h"
#include "comunicacion_worker.h"

volatile sig_atomic_t status=0;
void instance_signal_handler(void);
static void catch_handler_termination(int sign){
    log_warning(logger, "Handle termination");
    for(int i=0;i<list_size(workers);i++)
    {
        worker* w = (worker*)list_get(workers, i);
        if(w != NULL){
            close(w->fd);
        }
    }
    exit(EXIT_SUCCESS);
}
void* attend_multiple_clients(void* params);
void* go_loop_net(void* params);
void packet_callback(void* params);
void disconnect_callback(void* params);
worker* get_worker_by_id(int id);


