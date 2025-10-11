#include "base.h"
#include "signal.h"
//#include "funciones_generales.h"
#include "inicializacion_storage.h"
#include "control_accesos.h"
#include "comunicacion_worker.h"

volatile sig_atomic_t status=0;
void instance_signal_handler(void);
void* attend_multiple_clients(void* params);
void* go_loop_net(void* params);
void packet_callback(void* params);
void disconnect_callback(void* params);
worker* get_worker_by_id(int id);


