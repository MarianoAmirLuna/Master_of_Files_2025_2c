#ifndef STORAGE_BASE_H
#define STORAGE_BASE_H

#include "inc/common.h"
#include "pthread.h"
#include "modules/sockets/network.h"

#include <sys/stat.h>
#include <fcntl.h> 
#include <unistd.h>
#include <sys/mman.h>
#include "inc/libs.h"
#include "exts/array_ext.h"
#include "exts/list_ext.h"
#include "math.h"

#include "communications_ops/inc_comms_ops.h"


/*#ifndef EXTS_FILE_EXT_H
#include "exts/file_ext.h"
#endif */

/*int g_block_size;
int g_fs_size;
t_config* g_archivo_hash;

t_list* workers;

// cosas del bitmap
t_bitarray* g_bitmap;
int g_bitmap_fd;
size_t g_bitmap_size;


config_storage cs;
op_code_module itself_ocm;*/


#endif