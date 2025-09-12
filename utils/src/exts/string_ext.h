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
/*
* @brief Splitea con el ":"
* @param instr 
* @param tag 
* @param file 
* 
* Ejemplo de uso:
* @code 
* char* file=string_new();
* char* tag= string_new();;
* get_tag_file(instr, file,tag);
* log_orange(logger, "File: %s, Tag: %s", file, tag);
* free(file);
* free(tag);
* @endcode
*/
void get_tag_file(char* instr, char* file,char* tag){
    char** spl= string_split(instr, ":");
    strcpy(file, spl[0]);
    strcpy(tag, spl[1]);
    string_array_destroy(spl);
}

void get_space_instr(char* instr, char* left_side,char* right_side){
    char** spl= string_split(instr, " ");
    strcpy(left_side, spl[0]);
    strcpy(right_side, spl[1]);
    string_array_destroy(spl);
}


/// @brief Se espera que sea con 2 espacios: "MATERIAS:BASE 0 SISTEMAS_OPERATIVOS"
/// @param instr 
/// @param left_side 
/// @param middle_side 
/// @param right_side 
void get_two_space_instr(char* instr, char* left_side, char* middle_side ,char* right_side){
    char** spl= string_split(instr, " ");
    strcpy(left_side, spl[0]);
    strcpy(middle_side, spl[1]);
    strcpy(right_side, spl[2]);
    string_array_destroy(spl);
}
void get_both_num_instr(char* instr, int* a, int* b)
{
    char* sa = string_new();
    char* sb = string_new();
    get_space_instr(instr, sa,sb);
    *a = atoi(sa);
    *b = atoi(sb);
    free(sa);
    free(sb);
}

void remove_new_line(char* line){
    int len = strlen(line);
    if(len <= 0 || line[len-1] != '\n')
        return;
    line[len-1] = '\0';
}

#endif