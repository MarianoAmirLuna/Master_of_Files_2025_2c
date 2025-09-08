#ifndef STORAGE_BASE_H
#define STORAGE_BASE_H

#include "inc/common.h"
#include "pthread.h"
#include "modules/sockets/network.h"

#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"
/*#ifndef EXTS_FILE_EXT_H
#include "exts/file_ext.h"
#endif */

int g_block_size;
int g_fs_size;
t_config* g_archivo_hash;

config_storage cs;
op_code_module itself_ocm;


#endif