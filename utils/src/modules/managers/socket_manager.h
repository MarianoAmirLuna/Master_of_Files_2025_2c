#ifndef SOCKET_MANAGER_H
#define SOCKET_MANAGER_H

//El socket manager es extremamente útil para el manejo de múltiples de clientes como Kernel (con tantas IO) igual que en Memoria (con tantas IO, 1 CPU y 1 Kernel)
#include "commons/collections/dictionary.h"

#ifndef UTILS_ENUMS_H
#include "../../utils/enums.h"
#endif
#include "exts/common_ext.h"
#include <string.h>
t_list* list_sockets = NULL;
t_dictionary* sockets_dict= NULL;
typedef struct ss
{
    int sock;
    int is_server_side;
    /// @brief Para interfaces
    int is_focus;
    op_code_module module;
    /// @brief Usar Buffer_to_String y viceversa
    char* name;
}socket_structure;

socket_structure* create_socket_structure(){
    socket_structure* ss = malloc(sizeof(socket_structure));
    ss->name = string_new();
    return ss;
}

/// @brief [PRIVATE]
/// @param ss 
/// @return N/A
void* serialize_socket_structure(socket_structure* ss)
{
    void* data = malloc(sizeof(int)*4);
    int offset =0;
    memcpy(data, &ss->sock, sizeof(int));
    offset+=sizeof(int);
    memcpy(data+offset, &ss->is_server_side, sizeof(int));
    offset+=sizeof(int);
    memcpy(data+offset, &ss->is_focus, sizeof(int));
    offset+=sizeof(int);
    memcpy(data+offset, &ss->module, sizeof(int));
    /*int sz = 0;
    void* buf = string_to_buffersize(ss->name, &sz);
    memcpy(data+offset, buf, sz);*/
    return data;
}

/// @brief [PRIVATE]
/// @param data 
/// @return N/A
socket_structure* deserialize_socket_structure(void* data){
    socket_structure* res = malloc(sizeof(socket_structure));
    int offset =0;
    memcpy(&res->sock, data, sizeof(int));
    offset+=sizeof(int);
    memcpy(&res->is_server_side, data+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&res->is_focus, data+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&res->module, data+offset, sizeof(int));
    return res;
}

t_list* get_sockets_by_ocm(op_code_module ocm){
    if(sockets_dict == NULL)
        return NULL;
    t_list* res = list_create();
    t_list* keys =dictionary_keys(sockets_dict);
    for(int i=0;i<list_size(keys);i++){
        char* keyname = (char*)list_get(keys, i);
        socket_structure* ss = deserialize_socket_structure(dictionary_get(sockets_dict, keyname));
        if(ss->module ==ocm)
            list_add(res, ss);
    }
    return res;
}

socket_structure* get_socket(char* name){
    if(sockets_dict == NULL)
        return NULL;
    if(dictionary_has_key(sockets_dict, name))
        return deserialize_socket_structure(dictionary_get(sockets_dict, name));
    return NULL;
}

int get_socket_int(char* name){
    socket_structure* ss = get_socket(name);
    return ss->sock;
}
int get_socket_int_by_ocm(op_code_module ocm){
    socket_structure* ss = get_socket(ocm_to_string(ocm));
    if(ss == NULL)
        return -1;
    return ss->sock;
}
void add_socket_structure_by_name(char* name, socket_structure* ss){
    void* data = serialize_socket_structure(ss);
    if(sockets_dict == NULL)
        sockets_dict = dictionary_create();
    dictionary_put(sockets_dict, name, data);
}

t_list* get_sockets_fds(){
    t_list* res = list_create();
    if(sockets_dict == NULL)
        return res;
    t_list* keys = dictionary_keys(sockets_dict);
    for(int i=0;i<list_size(keys);i++){
        
        void* data = dictionary_get(sockets_dict, list_get(keys, i));
        socket_structure* ss = deserialize_socket_structure(data);
        if(ss == NULL){
            log_error(logger, "Socket structure null %s:%d", __func__, __LINE__);
            continue;
        }
        void* aloc = malloc(sizeof(int));
        memcpy(aloc, &ss->sock, sizeof(int));
        list_add(res, aloc);
    }
    return res;
}

void add_socket_structure_by_name_ocm_sock_server(char* name, op_code_module ocm, int sock, int is_server_side){
   
    socket_structure* ss = malloc(sizeof(socket_structure));
    ss->is_server_side =is_server_side;
    ss->module = ocm;
    ss->sock = sock;
    ss->is_focus = 1;
    ss->name = NULL;
    void* data = serialize_socket_structure(ss);
    if(sockets_dict == NULL)
        sockets_dict = dictionary_create();
    dictionary_put(sockets_dict, name, data);
    free(ss);
}

/// @brief [PRIVATE]
/// @param elem 
void destroyer_element_socket(void* elem)
{
    socket_structure* ss =deserialize_socket_structure(elem);
    if(ss != NULL){
        log_info(logger, "Cerrando socket: %d", ss->sock);
        close(ss->sock);
    }
    if(ss->name)
        free(ss->name);
    free(ss);
    free(elem); //es solo socket_structure, no necesito limpiar los campos porque no son punteros
}

void remove_socket(char* name){
    
    dictionary_remove_and_destroy(sockets_dict, name, destroyer_element_socket);
}
void remove_socket_by_module(op_code_module module){
    t_list* keys = dictionary_keys(sockets_dict);
    int len = list_size(keys);
    for(int i=0;i<len;i++){
        char* name = (char*)list_get(keys, i);
        socket_structure* ss = deserialize_socket_structure(dictionary_get(sockets_dict, name));
        if(ss == NULL)
            continue;
        if(ss->module == module){
            remove_socket(name);
            return;
        }
    }
}

void instance_list_sockets(){
    list_sockets = list_create();
}

void add_socket_list(int sock){
    list_add(list_sockets, sock);
}
void close_all_sockets_by_list(){
    for(int i=0;i<list_size(list_sockets);i++){
        int sock = (int)list_get(list_sockets, i);
        log_debug(logger, "Cerrando socket: %d", sock);
        close(sock);
    }
}

/// @brief [PRIVATE]
/// @param name 
/// @param elem 
void clear_iter(char* name, void* elem){
    free(elem);
}

void clear_sockets(){
    dictionary_iterator(sockets_dict, &clear_iter);
}

void close_all_sockets(){
    if(sockets_dict == NULL)
        return;
    dictionary_clean_and_destroy_elements(sockets_dict, destroyer_element_socket);
}
#endif