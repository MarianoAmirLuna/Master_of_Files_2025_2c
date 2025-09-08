#include "base.h"
#include "funciones_generales.h"
#include "inicializacion_storage.h"

void* attend_multiple_clients(void* params);
void* go_loop_net(void* params);
void packet_callback(void* params);
void disconnect_callback(void* params);
void tratar_mensaje(t_list* p, int cliente);
void ejecutar_storage_instruction(storage_operation so, char* par1, char* par2);


