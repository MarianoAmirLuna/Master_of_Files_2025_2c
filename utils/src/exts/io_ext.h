#ifndef IO_EXT_H
#define IO_EXT_H


#include <sys/stat.h>
#include <sys/mman.h>
#include "commons/string.h"

#ifndef INC_LIBS_H
#include "inc/libs.h"
#endif 

//#include "file_ext.h"
#include "bitmap_ext.h"

#define MAX_PATH 255

void crear_directorio(char* nombre, char* path)
{
    char* dir = string_from_format("%s/%s", path, nombre);    
    if(mkdir(dir, 0777) == 0) {
        log_pink(logger, "Se creo el directorio %s, en el path %s", nombre, path); //to_do: borrar este
    }
    free(dir);
}


bool control_existencia_file_old(const char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL)
    {
        return false;
    }
    log_warning(logger, "path: %s", path);
    FILE * archivo = fopen(path, "r");
    if (archivo != NULL)
    {
    //Path encontrado
        log_info(logger, "existe el archivo");
        fclose(archivo);
        return true;
    }
    else 
    {
        log_error(logger, "archivo no encontrado");
        //Path no encontrado
        return false;
    }
}

int control_existencia(char* path){
    //Controlo que no se me envie un path vacio
    if (path==NULL)
    {
        return false;
    }
    struct stat statbuf;
    if (stat(path, &statbuf)==0)
    {
        //Path encontrado
        return true;
    }
    else 
    {
        //Path no encontrado
        return false;
    }
}

int control_existencia_file(char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL){
        return 0;
    }
    log_warning(logger, "path: %s", path);
    FILE * archivo = fopen(path, "r");
    if (archivo != NULL)
    {
        //Path encontrado
        log_info(logger, "existe el archivo");
        fclose(archivo);
        return 1;
    }
    else 
    {
        log_error(logger, "archivo no encontrado");
        //Path no encontrado
        return 0;
    }
}

bool tag_comiteado(char* file, char* tag){
    load_config(string_to_format("files/%s/%s/metadata.config", file, tag));
    
}


int cant_elementos_directorio(char* path){
    int cnt = 0;
    DIR* dir = opendir(path);
    if(!dir)
    {
        log_error(logger, "No se pudo abrir el directorio %s", path);
        return -1;
    }
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL){
        if (!string_equals_ignore_case(entry->d_name, ".") && !string_equals_ignore_case(entry->d_name, "..")){
            cnt++;
        }
    }

    closedir(dir);
    return cnt;
}

int crear_archivo(char* path, char* nombre, char *extension){
    char* p_path = string_from_format("%s/%s.%s", path, nombre, extension);
    if (!control_existencia(p_path))
    {
        FILE * archivo = fopen(p_path, "w");
        fclose(archivo);
    }
    else
    {
        log_warning(logger, "El archivo %s.%s ya existe en el path %s", nombre, extension, path);
    }
    free(p_path);
}

void eliminar_archivo (char* path, char* nombre)
{
    char* p_path = string_from_format("%s/%s", path, nombre);
    if (control_existencia(p_path)) {
        remove(p_path);   // ✅ recibe const char* //El ✅ es 0 es elemento nulo
    } else {
        log_error(logger, "El archivo %s no existe en el path %s", nombre, path);
    }
    free(p_path);
}



void crear_hard_link(char* path_bloque_fisico, char* path_bloque_logico)
{
    if(link(path_bloque_fisico, path_bloque_logico) == 0)
    {
        log_debug(logger, "se vinculo el bloque del path %s, con el bloque en el path %s", path_bloque_logico, path_bloque_fisico);
    }
    else
    {
        perror("link");
        log_error(logger, "ERROR EN VINCULACION DE BLOQUES LOGICO: %s, FISICO: %s", path_bloque_logico, path_bloque_fisico);
    }
}

void crear_config_path(char* p_path, char* name)
{
    char* path = string_from_format("%s/%s", p_path, name);
    create_blocks_hash_index(path);
    free(path);
}


t_config* crear_metadata_config(char* path, int tamanio, t_list* bloques, state_metadata estado) {
    // path debe terminar en /metadata.config
    t_config* metadata = config_create(path);
    if (metadata == NULL) {
        metadata = malloc(sizeof(t_config));
        metadata->path = string_duplicate(path);
        metadata->properties = dictionary_create();
    }

    // TAMAÑO
    config_set_value(metadata, "TAMAÑO", string_itoa(tamanio));

    // BLOCKS → construir string "[0,1,2,...]"
    char* bloques_str;
    if (list_is_empty(bloques)) {
        bloques_str = strdup("[]");
    } else {
        char* tmp = string_new();
        string_append(&tmp, "[");
        for (int i = 0; i < list_size(bloques); i++) {
            int* b = list_get(bloques, i);
            char* num = string_itoa(*b);
            string_append(&tmp, num);
            free(num);
            if (i < list_size(bloques) - 1) string_append(&tmp, ",");
        }
        string_append(&tmp, "]");
        bloques_str = tmp;
    }
    config_set_value(metadata, "BLOCKS", bloques_str);
    free(bloques_str);

    // ESTADO
    const char* estado_str = (estado == WORK_IN_PROGRESS) ? "WORK_IN_PROGRESS" : "COMMITED";
    config_set_value(metadata, "ESTADO", (char*)estado_str);

    config_save(metadata);
    return metadata;
}

void llenar_archivo_con_ceros(char* path_archivo, int g_block_size) 
{
    FILE* archivo = fopen(path_archivo, "wb");
    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo %s para llenarlo", path_archivo);
        return;
    }

    // buffer temporal con '0'
    char* buffer = malloc(g_block_size);
    if (!buffer) {
        log_error(logger, "No se pudo reservar memoria para llenar %s", path_archivo);
        fclose(archivo);
        return;
    }

    memset(buffer, '0', g_block_size);  // llenar con el caracter '0'
    fwrite(buffer, 1, g_block_size, archivo);

    free(buffer);
    fclose(archivo);
}

void eliminar_bloques_fisicos (int cantidad_bloques_de_mas, char* path, int bloques_actuales)
{
    // Elimino los bloques fisicos que sobren
    for (int i = 0; i < cantidad_bloques_de_mas; i++)
    {
        char* p_path = string_from_format("block%04d.dat", bloques_actuales-i);
        eliminar_archivo(path, p_path);
        free(p_path);
    }
}

void crear_bloques_fisicos (int cantidad_bloques_faltantes, char* path, int bloques_actuales, int g_block_size)
{
    for (int i = 0; i < cantidad_bloques_faltantes; i++)
    {
        char* nombre = string_from_format("block%04d", bloques_actuales + i);

        crear_archivo(path, nombre, "dat");

        char* full_path = string_from_format("%s/%s.dat", path, nombre);
        llenar_archivo_con_ceros(full_path, g_block_size);

        free(nombre);
        free(full_path);
    }
}

void escribir_bloque_fisico(int bloque_fisico, char* contenido)
{
    char* path = string_from_format("%s/physical_blocks/block%04d.dat",cs.punto_montaje, bloque_fisico);

    FILE* f = fopen(path, "r+");

    if( f = NULL )
    {
        log_error(logger, "[WRITE_BLOCK] no se pudo abrir el bloque fisico %d en %s", bloque_fisico, path);
        free(path);
        return;
    }
}


#endif
