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
#ifndef EXTS_STRING_H
#include "exts/string_ext.h"
#endif 
#ifndef EXTS_LIST_EXT
#include "exts/list_ext.h"
#endif

#ifndef CAST_EXT_H
#include "../exts/cast_ext.h"
#endif
#ifndef CRYPTO_H_
#include "commons/crypto.h"
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
    cw.path_queries=config_get_string_value(config, "PATH_SCRIPTS");
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
        printf("No se encontro el config %s asi que se creo uno nuevo\n", path);
        config = malloc(sizeof(t_config));
        config->path = strdup(path);
        config->properties = dictionary_create();
        /*printf("ERROR no se pudo cargar el config");
        exit(1);*/
    }
    return config;
}


t_config* create_super_block(char* path, int fs_size, int block_size)
{
    if(fs_size < 0 || block_size < 0){
        printf("%s, (%s:%d)","WHAT THE FUCK fs_size o block_size menores o iguales a 0???? EXIT(1) IS INVOKED", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }

    t_config* superblock = load_config(path);
    config_set_value(superblock, "FS_SIZE", string_itoa(fs_size));
    config_set_value(superblock, "BLOCK_SIZE", string_itoa(block_size));    
    config_save(superblock);
    return superblock;
}

int get_fs_size_superblock(t_config* superblock){
    return config_get_int_value(superblock, "FS_SIZE");
}

int get_block_size_superblock(t_config* superblock){
    return config_get_int_value(superblock, "BLOCK_SIZE");
}

t_config* create_blocks_hash_index(char* path){
    t_config* config = load_config(path);
    config_save(config);
    return config;
}

t_config* load_block_hash(char* path){
    return config_create(path);
}

/// @brief Recibe lista de hash (NO EL NOMBRE DEL BLOQUE SOLO EL HASH)
/// @param block_hash_index 
/// @return 
t_list* get_all_hash_of_block_hash(t_config* block_hash_index){
    return dictionary_keys(block_hash_index->properties);
}

t_list* get_all_value__of_block_hash(t_config* block_hash_index){
    t_list* res = list_create();
    t_list* keys = get_all_hash_of_block_hash(block_hash_index);
    for(int i=0;i<list_size(keys);i++){
        char* value =dictionary_get(block_hash_index->properties, (char*)list_get(keys, i));
        list_add(res, value);
    }
    return res;
}

bool exists_hash_in_block_hash(t_config* block_hash_index, char* hash){
    return config_has_property(block_hash_index, hash);
}

t_config* insert_hash_block(t_config* block_hash_index, char* hash, char* block){
    //Lo gracioso es que el MD5 es un hash pobre y es más propenso a tener colisión de hash que un SHA1
    if(block_hash_index == NULL){
        printf("%s (%s:%d)", "El Config block hash index no está creado...", __func__,__LINE__);
        exit(EXIT_FAILURE);
    }

    config_set_value(block_hash_index, hash, block);
     if(config_save(block_hash_index) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en (%s:%d)", __func__, __LINE__);
    }
    return block_hash_index;
}

t_config* insert_hash_block_n(t_config* block_hash_index, char* hash, int nblock){
    char* blockname = get_block_name_by_n(nblock, NUMBER_OF_DIGITS_BLOCK);
    t_config* res = insert_hash_block(block_hash_index, hash, blockname);
    free(blockname);
    return res;
}

t_config* insert_crypto_hash_block(t_config* block_hash_index, void* value, int len_value, char* block){
    //Lo gracioso es que el MD5 es un hash pobre y es más propenso a tener colisión de hash que un SHA1
    if(block_hash_index == NULL){
        printf("%s (%s:%d)", "El Config block hash index no está creado...", __func__,__LINE__);
        exit(EXIT_FAILURE);
    }
    if(len_value == 0 || value == NULL)
    {
        log_error(logger, "WTF el valor a hashear es nulo o de tamaño 0 (%s:%d)", __func__, __LINE__);
        exit(EXIT_FAILURE);
    }
    char* hash = crypto_md5(value, len_value);
    config_set_value(block_hash_index, hash, block);
    if(config_save(block_hash_index) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en (%s:%d)", __func__, __LINE__);
    }
    free(hash);
    return block_hash_index;
}

t_config* insert_crypto_hash_block_n(t_config* block_hash_index, void* value, int len_value, int nblock){
    char* block_name = get_block_name_by_n(nblock, NUMBER_OF_DIGITS_BLOCK);
    t_config* res = insert_crypto_hash_block(block_hash_index, value, len_value, block_name);
    free(block_name);
    return res;
}
t_config* create_metadata(char* path, int size, t_list* blocks, state_metadata state){

    //t_config* metadata = config_create(path);
    t_config* metadata = load_config(path);

    int blocks_is_null_or_empty = blocks == NULL || list_is_empty(blocks);
    config_set_value(metadata, "TAMAÑO", string_itoa(size));
    config_set_value(metadata, "BLOCKS", blocks_is_null_or_empty ? "[]" : list_array_int_as_string_v2(blocks));
    config_set_value(metadata, "ESTADO", get_string_state(state));
    if(config_save(metadata) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en %s (%s:%d)", path, __func__, __LINE__);
    }
    return metadata;
}

void set_state_metadata(char* path, state_metadata state){
    t_config* metadata = load_config(path);
    config_set_value(metadata, "ESTADO", get_string_state(state));
    if(config_save(metadata) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en %s (%s:%d)", path, __func__, __LINE__);
    }
    config_destroy(metadata);
}

/// @brief Setea el metadata pero no destruye el puntero t_config
/// @param conf 
/// @param state 
void set_state_metadata_from_config(t_config* conf, state_metadata state){
    config_set_value(conf, "ESTADO", get_string_state(state));
    if(config_save(conf) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en %s (%s:%d)", conf->path, __func__, __LINE__);
    }
}

/// @brief Debe ser liberado con un string_array_destroy cuando ya no se usa
/// @param metadata 
/// @return 
char** get_array_blocks_from_metadata(t_config* metadata){
    return config_get_array_value(metadata, "BLOCKS");
}

t_list* get_array_blocks_as_list_from_metadata(t_config* metadata){
    char** array = get_array_blocks_from_metadata(metadata);
    t_list* res = list_create();
    int len = string_array_size(array);
    for(int i=0;i<len;i++){
        list_add(res, atoi(array[i]));
    }
    string_array_destroy(array);
    return res;
}

int get_size_from_metadata(t_config* metadata){
    return config_get_int_value(metadata, "TAMAÑO");
}

/// @brief `[PRIVATE]`
/// @param metadata 
/// @param blocks 
void _set_blocks_metadata(t_config* metadata, t_list* blocks){
    char* block_as_str = list_array_int_as_string_v2(blocks);
    config_set_value(metadata, "BLOCKS", block_as_str);
    free(block_as_str);
    if(config_save(metadata) == -1){
        log_error(logger, "Hubo un error no se pudo guardar el config en %s (%s:%d)", metadata->path, __func__, __LINE__);
    }
    list_destroy(blocks);
}

int remove_block_from_metadata(t_config* metadata, int block_number){
    char** blocks_array = get_array_blocks_from_metadata(metadata);
    int len = string_array_size(blocks_array);
    t_list* blocks_list = list_create();
    for(int i=0;i<len;i++){
        int bn = atoi(blocks_array[i]);
        if(bn != block_number){
            list_add(blocks_list, bn);
        }
    }
    string_array_destroy(blocks_array);
    _set_blocks_metadata(metadata, blocks_list);
    return 0;
}

int insert_block_from_metadata(t_config* metadata, int block_number){
    t_list* blocks_list = get_array_blocks_as_list_from_metadata(metadata);
    if(list_contain_int(blocks_list, block_number)){
        log_warning(logger, "El bloque %d ya existe en el metadata %s, no se inserta de nuevo (%s:%d)", block_number, metadata->path, __func__, __LINE__);
    }
    list_add(blocks_list, block_number);
    _set_blocks_metadata(metadata, blocks_list);
    return 0;
}

state_metadata get_state_metadata(t_config* metadata){
    char* estado_str = config_get_string_value(metadata, "ESTADO");
    return cast_state_metadata(estado_str);
}


/// @brief  Obtiene el cs.montaje/files/$file$/$tag$
/// @param cs 
/// @param file 
/// @param tag 
/// @return 
char* get_filetag_path(config_storage cs, char* file, char* tag){
    return string_from_format("%s/files/%s/%s", cs.punto_montaje, file, tag);
}

char* get_metadata_fullpath(config_storage cs, char* file, char* tag){
    char* filetag =get_filetag_path(cs, file, tag);
    char* fullpath = string_from_format("%s/metadata.config", filetag);
    free(filetag);
    return fullpath;
}
t_config* get_metadata_from_file_tag(config_storage cs, char* file, char* tag){
    
    char* fullpath = get_metadata_fullpath(cs,file,tag);
    t_config* res= load_config(fullpath);
    free(fullpath);
    return res;
}
t_config* get_block_hash_index(config_storage cs){
    char* fullpath = string_from_format("%s/blocks_hash_index.config", cs.punto_montaje);
    t_config* res= load_config(fullpath);
    free(fullpath);
    return res;
}

/// @brief Obtiene el cs.montaje/files/$file$/$tag$/logical_blocks
/// @param cs 
/// @param file 
/// @param tag 
/// @return 
char* get_logical_blocks_dir(config_storage cs, char* file, char* tag){
    return string_from_format("%s/files/%s/%s/logical_blocks", cs.punto_montaje, file, tag);
}

/// @brief Obtiene el cs.montaje/physical_blocks
/// @param cs 
/// @return 
char* get_physical_blocks_dir(config_storage cs){
    return string_from_format("%s/physical_blocks", cs.punto_montaje);
}
/// @brief 
/// @brief Obtiene el cs.montaje/files
/// @param cs 
/// @return 
char* get_files_from_punto_montaje(config_storage cs){
    return string_from_format("%s/files", cs.punto_montaje);
}

#endif