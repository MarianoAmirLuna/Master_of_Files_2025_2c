#include "base.h"
#include "signal.h"

volatile sig_atomic_t status=0;
void instance_signal_handler(void);

void* go_loop_net(void* params);
void packet_callback(void* params);
void* attend_multiple_clients(void* params);
void disconnect_callback(void* params);