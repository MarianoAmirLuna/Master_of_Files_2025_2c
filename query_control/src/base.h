#ifndef QUERY_CONTROL_BASE_H
#define QUERY_CONTROL_BASE_H

#include "inc/common.h"
#include "modules/sockets/network.h"

#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"

config_query_control cqc;
op_code_module itself_ocm;
int priority=0;
char* archive_query;


#endif