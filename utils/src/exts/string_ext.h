#ifndef EXTS_STRING_H
#define EXTS_STRING_H

#include <inc/common.h>
#include <commons/string.h>

#define NUMBER_OF_DIGITS_BLOCK 4

/// @brief `get_name_fmt_number("block_", 5, 6);` -> `block_000005`
/// @param name 
/// @param n 
/// @param total_digits 
/// @return 
char* get_name_fmt_number(char* name, int n, int total_digits){
    char* nn = string_new();
    sprintf(nn, "%s%0*d", name, total_digits, n);
    return nn;
}

/// @brief `get_name_extension_fmt_number("file_", "txt", 5, 6);` -> `file_000005.txt`
/// @param name 
/// @param extension 
/// @param n 
/// @param total_digits 
/// @return 
char* get_name_extension_fmt_number(char* name, char* extension, int n, int total_digits){
    char* nn = string_new();
    sprintf(nn, "%s%0*d.%s", name, total_digits, n, extension);
    return nn;
}

/// @brief Cuando ya no se usa liberar con free()
/// `get_block_name(5);` -> `block0005`
/// @param nblock Número de bloque. La cantidad está difinida por macro `NUMBER_OF_DIGITS_BLOCK`
/// @return 
char* get_block_name(int nblock){
    char* block_name = string_new();
    sprintf(block_name, "block%0*d", NUMBER_OF_DIGITS_BLOCK, nblock);
    return block_name;
}
/// @brief Se tiene  XXXXXX.dat con 6 números.
/// @param n 
/// @return 
char* get_block_name_logical(int n){
    char* block_name = string_new();
    sprintf(block_name, "%0*d.dat", 6, n);
    return block_name;
}
/// @brief Se tiene el blockXXXX.dat con 4 números.
/// @param n 
/// @return 
char* get_block_name_physical(int n){
    char* block_name = string_new();
    sprintf(block_name, "block%0*d.dat", 4, n);
    return block_name;
}
/// @brief Cuando ya no se usa liberar con free()
/// `get_block_name_by_n(5, 6);` -> `block000005`
/// @param nblock El número de bloque
/// @param n_numbers Cantidad de digitos
/// @return 
char* get_block_name_by_n(int nblock, int n_numbers){
    char* block_name = string_new();
    sprintf(block_name, "block%0*d", n_numbers, nblock);
    return block_name;
}

//WARNING: USE MD5 OF CRYPTO LIBS COMMON DO NOT USE THIS
/*unsigned long hash(unsigned char* str){
    unsigned long hash = 5381;
    int c;
    while((c=*str++))
        hash=((hash<<5)+hash)+c;
    return hash;
}*/

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
    printf("Refer: %s", instr);
    get_space_instr(instr, sa,sb);
    *a = atoi(sa);
    *b = atoi(sb);
    free(sa);
    free(sb);
}

void remove_new_line(char* line){
    size_t len = strlen(line);
    if(len <= 0 || line[len-1] != '\n')
        return;
    line[len-1] = '\0';
}

#endif