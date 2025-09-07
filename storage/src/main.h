#include "base.h"
#include "signal.h"



volatile sig_atomic_t status=0;
void instance_signal_handler(void);

void* go_loop_net(void* params);
void packet_callback(void* params);
void* attend_multiple_clients(void* params);
void disconnect_callback(void* params);
void inicializar_file_system();
void tratar_mensaje(t_list* p, int cliente);
void ejecutar_storage_instruction(storage_operation so, char* par1, char* par2);
void valido_bitmap(const char* path);
void valido_hash(const char* path);
void valido_bloques_fisicos(const char* path);
void valido_inicial_file(char* path);
void inicializar_bitnap();
bool control_existencia(const char* path);
