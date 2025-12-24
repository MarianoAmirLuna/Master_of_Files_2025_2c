#ifndef BASE_COMMUNICATION_OPS_H
#define BASE_COMMUNICATION_OPS_H

#include "inc/libs.h"


int g_block_size;
int g_fs_size;
t_config* g_archivo_hash;

t_list* workers;

// cosas del bitmap
t_bitarray* g_bitmap;
int g_bitmap_fd;
size_t g_bitmap_size;


config_storage cs;
op_code_module itself_ocm;

//Se debe tomar como si fuera un encabezado o tendrás warning implicit-declaration y puede ser un problema al ejecutarlo
void eliminar_contenido(const char* path);

int file_tag_exist_or_not(char* file, char* tag, worker* w);
#ifndef IO_EXT_H
#include "exts/io_ext.h"
#endif 
#ifndef STORAGE_BASE_H
#include "../base.h"
#endif

#endif