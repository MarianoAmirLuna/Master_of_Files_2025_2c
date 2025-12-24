#include "base.h"
#include "funciones_worker.h"
#include "signal.h"

volatile sig_atomic_t status=0;
static void catch_handler_termination(int sign){
    log_warning(logger, "Handle termination");
    /*if(actual_worker != NULL && actual_worker->fd){
        close(actual_worker->fd);
    }*/
    close(sock_master);
    close(sock_storage);
    exit(EXIT_SUCCESS);
}
void instance_signal_handler(void);


void packet_callback(void* params);
void* connect_to_server(void* params);
void loop_atender_queries();