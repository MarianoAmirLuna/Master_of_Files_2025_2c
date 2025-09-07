#ifndef MASTER_BASE_H
#define MASTER_BASE_H

#ifndef SCHEDULER_H
#include "sched/scheduler.h"
#endif

op_code_module itself_ocm;
void work_query_control(t_list* packet, int sock_client);
void work_worker(t_list* packet, int sock_client);

#endif