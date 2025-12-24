#include "base.h"
#include "signal.h"
volatile sig_atomic_t status=0;
static void catch_handler_termination(int sign){
    log_warning(logger, "Handle termination");
    for(int i=0;i<list_size(workers);i++){
        worker* w = cast_worker(list_get(workers, i));
        if(w != NULL)
            close(w->fd);
    }
    for(int i=0;i<list_size(queries);i++){
        query* q = cast_query(list_get(queries, i));
        if(q != NULL)
            close(q->fd);
    }
    exit(EXIT_SUCCESS);
}
void instance_signal_handler(void);

void* go_loop_net(void* params);
void packet_callback(void* params);
void* attend_multiple_clients(void* params);
void disconnect_callback(void* params);
void* console();