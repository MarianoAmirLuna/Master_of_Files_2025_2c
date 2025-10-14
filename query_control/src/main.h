#include "base.h"

volatile sig_atomic_t status=0;
static void catch_handler_termination(int sign){
    log_warning(logger, "Handle termination");
    close(fd_master);
    exit(EXIT_SUCCESS);
}
void instance_signal_handler(void);