#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>

#include "../utils/enums.h"

#include "commons/collections/list.h"

#ifndef CONFIG_H_
#include <commons/config.h>
#endif
#ifndef LIBS_LOGGER_H
#include "logger.h"
#endif

#ifndef EXTS_LIST_EXT
#include "exts/list_ext.h"
#endif

#ifndef CAST_EXT_H
#include "../exts/cast_ext.h"
#endif
t_config* config;

typedef struct{
    char* ip_master;
    int puerto_master;
    t_log_level log_level;
}config_query_control;

typedef struct{
    int puerto_escucha;
    scheduler_algorithm algoritmo_planificacion;
    int tiempo_aging;;
    t_log_level log_level;
}config_master;


typedef struct{
    char* ip_master;
    int puerto_master;
    char* ip_storage;
    int puerto_storage;
    int tam_memoria;
    int retardo_memoria;
    replace_algorithm algoritmo_reemplazo;
    char* path_queries;
    t_log_level log_level;
}config_worker;

typedef struct{
    int puerto_escucha;
    int fresh_start;
    char* punto_montaje;
    int retardo_operacion;
    int retardo_acceso_bloque;
    t_log_level log_level;
}config_storage;

int get_boolean_config(char* value){
    return string_equals_ignore_case(value, "TRUE");
}

void check_null_config(){
    if(config != NULL)
        return;
    if(logger != NULL){
        log_error(logger, "Config nulo");
    }
    else{
        printf("Error config nulo %s:%d", __func__, __LINE__);
    }
    exit(1);
}
/// @brief [PRIVATE]
/// @return N/A
t_log_level get_log_level(){
    char* str = config_get_string_value(config, "LOG_LEVEL");
    return log_level_from_string(str);
}

void non_exists_config(char* nonexists){
    if(logger != NULL){
        log_error(logger,"%s",nonexists);
    }else{
        printf("%s\n", nonexists);
    }
    exit(1);
}

scheduler_algorithm get_scheduler_algorithm()
{
    char* sa = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
    if(string_equals_ignore_case(sa, "FIFO"))
        return FIFO;
    if(string_equals_ignore_case(sa, "PRIORIDADES"))
        return PRIORITIES;
    non_exists_config("No existe el algoritmo de planificacion");
    return FIFO;
}

replace_algorithm get_replace_algorithm(){
    char* sa = config_get_string_value(config, "ALGORITMO_REEMPLAZO");
    if(string_equals_ignore_case(sa, "LRU"))
        return R_LRU;
    if(string_equals_ignore_case(sa, "CLOCK-M"))
        return R_CLOCK_M;
    non_exists_config("No existe el algoritmo de reemplazo");
    return R_LRU;
}

/// @brief Carga los datos del query control
/// Nota: previamente el método load_config(path) tiene que ser llamado
/// @return config_query_control
config_query_control load_config_query_control(){
    check_null_config();
    config_query_control cqc;
    cqc.ip_master = config_get_string_value(config, "IP_MASTER");
    cqc.puerto_master = config_get_int_value(config, "PUERTO_MASTER");
    cqc.log_level = get_log_level();
    return cqc;
}

/// @brief Carga los datos del master
/// Nota: previamente el método load_config(path) tiene que ser llamado
/// @return config_master
config_master load_config_master(){
    check_null_config();
    config_master cm;
    cm.puerto_escucha =config_get_int_value(config, "PUERTO_ESCUCHA");
    cm.algoritmo_planificacion = get_scheduler_algorithm();
    cm.tiempo_aging =config_get_int_value(config, "TIEMPO_AGING");
    cm.log_level=get_log_level();
    return cm;
}

/// @brief Carga los datos del worker. 
/// Nota: previamente el método load_config(path) tiene que ser llamado
/// @return config_worker
config_worker load_config_worker(){
    check_null_config();
    config_worker cw;
    cw.ip_master=config_get_string_value(config, "IP_MASTER");
    cw.puerto_master=config_get_int_value(config, "PUERTO_MASTER");
    cw.ip_storage=config_get_string_value(config, "IP_STORAGE");
    cw.puerto_storage=config_get_int_value(config, "PUERTO_STORAGE");
    cw.tam_memoria=config_get_int_value(config, "TAM_MEMORIA");
    cw.retardo_memoria=config_get_int_value(config, "RETARDO_MEMORIA");
    cw.algoritmo_reemplazo = get_replace_algorithm();
    cw.path_queries=config_get_string_value(config, "PATH_QUERIES");
    cw.log_level = get_log_level();
    return cw;
}


/// @brief Carga los datos del memory. 
/// Nota: previamente el método load_config(path) tiene que ser llamado
/// @return config_memory
config_storage load_config_storage(){
    check_null_config();
    config_storage cs;
    cs.puerto_escucha=config_get_int_value(config, "PUERTO_ESCUCHA");
    cs.fresh_start=get_boolean_config(config_get_string_value(config, "FRESH_START"));
    cs.punto_montaje=config_get_string_value(config, "PUNTO_MONTAJE");
    cs.retardo_operacion=config_get_int_value(config, "RETARDO_OPERACION");
    cs.retardo_acceso_bloque=config_get_int_value(config, "RETARDO_ACCESO_BLOQUE");
    cs.log_level=get_log_level();
    return cs;
}
/// @brief Carga el archivo en variable global config
/// @param path Ruta del .config
/// @return retorna la variable  global config
t_config* load_config(char* path){
    config = config_create(path);
    if(config == NULL){
        printf("ERROR no se pudo cargar el config");
        exit(1);
    }
    return config;
}

t_config* create_super_block(char* path, int fs_size, int block_size)
{
    if(fs_size < 0 || block_size < 0){
        printf("%s, (%s:%d)","WHAT THE FUCK fs_size o block_size menores o iguales a 0???? EXIT(1) IS INVOKED", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    t_config* superblock = config_create(path);
    if(superblock == NULL){
        superblock = malloc(sizeof(t_config));
        superblock->path = strdup(path);
    }
    config_set_value(superblock, "FS_SIZE", string_itoa(fs_size));
    config_set_value(superblock, "BLOCK_SIZE", string_itoa(block_size));
    
    config_save(superblock);
    return superblock;
}

t_config* create_blocks_hash_index(char* path){
    t_config* config = malloc(sizeof(t_config));
    config->path = strdup(path);
    config_save(config);
    return config;
}

t_config* insert_hash_block(t_config* block_hash_index, char* hash, char* block){
    //Lo gracioso es que el MD5 es un hash pobre y es más propenso a tener colisión de hash que un SHA1
    if(block_hash_index == NULL){
        printf("%s (%s:%d)", "El Config block hash index no está creado...", __func__,__LINE__);
        exit(EXIT_FAILURE);
    }

    config_set_value(block_hash_index, hash, block);
    return block_hash_index;
}
t_config* create_metadata(char* path, int size, t_list* blocks, state_metadata state){

    t_config* metadata = config_create(path);

    if(metadata == NULL){
        //https://github.com/sisoputnfrba/so-commons-library/blob/master/src/commons/config.c
        metadata = malloc(sizeof(t_config));
        metadata->path = strdup(path);
        config_save(metadata);
        metadata = config_create(path);
    }
    if(blocks == NULL || list_is_empty(blocks))
    {
        printf("%s (%s:%d)", "La lista blocks está vacía o es nula", __func__,__LINE__);
        exit(EXIT_FAILURE);
    }
    config_set_value(metadata, "TAMAÑO", string_itoa(size));
    config_set_value(metadata, "BLOCKS", list_array_int_as_string(blocks));
    config_set_value(metadata, "ESTADO", get_string_state(state));
    return metadata;
}
#endif