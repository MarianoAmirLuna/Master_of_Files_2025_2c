#ifndef PSEUDOCODE_MANAGER_H
#define PSEUDOCODE_MANAGER_H

#define MAXLEN 256

#ifndef UTILS_STRUCTS_H
#include "../../utils/structs.h"
#endif
#include "../../libs/logger.h"
#ifndef EXTS_STRING_H
#include "../../exts/string_ext.h"
#endif

#ifndef EXTS_COMMON_EXT
#include "../../exts/common_ext.h"
#endif
t_list* list_pseudocode=NULL;

pseudocode* create_pseudocode(){
    pseudocode* result = malloc(sizeof(pseudocode));
    result->pid = -1;
    result->fullpath = calloc(255, sizeof(char));
    result->filename = calloc(255, sizeof(char));
    result->instructions =list_create();
    return result;
}

void free_pseudocode(pseudocode* pseudo){
    for(int i=0;i<list_size(pseudo->instructions);i++){
        pseudocode* p =(pseudocode*)list_get(pseudo->instructions, i);
        if(p->pid == pseudo->pid){
            free(p->filename);
            free(p->fullpath);
            list_destroy_and_destroy_elements(p->instructions, free);
            free(p);
            return;
        }
    }
}

pseudocode* get_pseudo(pid_t pid){
    if(list_pseudocode == NULL)
        return NULL;
    for(int i=0;i<list_size(list_pseudocode);i++){
        pseudocode* p =(pseudocode*)list_get(list_pseudocode, i);
        if(p->pid == pid)
            return p;
    }
    return NULL;
}

int exists_pseudo(pid_t pid){
    return get_pseudo(pid) != NULL;
}

/// @brief Esto no agrega pseudocódigo use `add_pseudocode`
/// @param pid 
/// @param tid 
/// @param fullpath 
/// @return 
pseudocode* load_pseudocode(pid_t pid, char* fullpath){
    if(exists_pseudo(pid)){
        log_warning(logger, "PID ya existe llamada en %s:%d", __func__, __LINE__);
        return get_pseudo(pid);
    }
    if(!file_exists(fullpath)){
        log_error(logger, "El archivo \"%s\" no existe", fullpath);
        return NULL;
    }
    FILE* f = fopen(fullpath, "r");
    if(f == NULL){
        log_error(logger, "No se pudo abrir el archivo en %s:%d", __func__, __LINE__);
        return NULL;
    }    
    pseudocode* p = create_pseudocode();
    p->pid = pid;
    strcpy(p->fullpath, fullpath);
    char* line = NULL;
    size_t len=0;
    ssize_t read;
    
    while((read = getline(&line, &len, f)) != -1){
        if(string_is_empty(line))
            break;
        list_add(p->instructions, string_to_buffer(line));
        if(feof(f))
            break;
    }
    fclose(f);
    if(line)
        free(line);
    return p;
}

void add_pseudocode(pid_t pid, char* fullpath){
    if(list_pseudocode == NULL)
        list_pseudocode = list_create();

    if(exists_pseudo(pid))
        return;
    char* path = malloc(strlen(fullpath)+1);
    strcpy(path, fullpath);
    list_add(list_pseudocode, (void*)load_pseudocode(pid, path));
    free(path);
    
}

void free_pseudocode(pid_t pid){
    pseudocode*  pseudo = get_pseudo(pid);
    if(pseudo == NULL)
        return;
    list_clean_and_destroy_elements(pseudo->instructions, free);
    free(pseudo);
}

/// @brief Busca el Pseudocodigo si lo encontró busca la linea de instrucción mediante PC (o program counter)
/// @param pid PID del proceso
/// @param pc Program counter
/// @return Línea de instrucción
char* get_line_by_pc(pid_t pid, uint32_t pc){
    if(!exists_pseudo(pid))
    {
        log_error(logger, "El pseudocode del %d no existe en PC: %d", pid, pc);
        return NULL;
    }
    t_list* instrs = get_pseudo(pid)->instructions;
    int sz = list_size(instrs);
    if(sz == pc)
        return NULL;
    if(sz < pc)
    {
        log_error(logger, "Out-of-index %s:%d", __func__, __LINE__);
        return NULL;
    }
    return buffer_to_string(list_get(instrs, pc));
}

#endif