#ifndef EXTS_STRING_H
#define EXTS_STRING_H

#include <inc/common.h>
#include <commons/string.h>

//WARNING: USE MD5 OF CRYPTO LIBS COMMON DO NOT USE THIS
unsigned long hash(unsigned char* str){
    unsigned long hash = 5381;
    int c;
    while((c=*str++))
        hash=((hash<<5)+hash)+c;
    return hash;
}

/// @brief Convierte la cadena en void*
/// @param str Cadena
/// @return Buffer
void* string_to_buffer(char* str){
    int len = strlen(str)+1;
    void* v = malloc(len +sizeof(int));
    memcpy(v, &len, sizeof(int));
    memcpy(v+sizeof(int), str, len);
    return v;
}

/// @brief Convierte  la cadena en void*
/// @param str Cadena
/// @param sz Puntero de tamaño de buffer, se usa en casos especiales
/// @return Buffer
void* string_to_buffersize(char* str, int* sz){
    int len = strlen(str)+1;
    int offset = sizeof(int);
    void* v = malloc(len+offset);
    memcpy(v, &len, sizeof(int));
    memcpy(v+offset, str, len);
    *sz = len + sizeof(int);
    return v;
}

/// @brief Convierte el buffer en cadena
/// @param buf Buffer
/// @return Cadena
char* buffer_to_string(void* buf){
    int len = 0;
    memcpy(&len, buf, sizeof(int));
    if(len <= 0)
        return NULL;
    char* c = (char*)malloc(len);
    int offset = sizeof(int);
    memcpy(c, buf +offset, len);
    return c;
}

#endif