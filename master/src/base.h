#ifndef MASTER_BASE_H
#define MASTER_BASE_H

#ifndef SCHEDULER_H
#include "sched/scheduler.h"
#endif

op_code_module itself_ocm;
/// @brief Es como PID, es incremental desde 0, doc página 13
int query_idx=0;

#endif