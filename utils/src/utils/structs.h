#ifndef UTILS_STRUCTS_H
#define UTILS_STRUCTS_H

#include <unistd.h>
#include "typedef.h"
#ifndef UTILS_ENUMS_H
#include "enums.h"
#endif

#include "commons/collections/list.h"
#include "commons/collections/queue.h"
#include "commons/bitarray.h"
#include "commons/temporal.h"
//#include "exts/temporal_ext.h"
#define MAX_INPUT 255

typedef struct
{
	int size;
	void *stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer *buffer;
} t_paquete;

typedef struct{
    /// @brief Código de operación
    op_code opcode;
    /// @brief Buffer
    t_buffer* buffer;
}t_packet;

typedef struct
{
    instr_code icode;
    int sz_args;
    datatype* types;
    void** args;
    char* str; //Por posibles problemas con void** prefiero prevenir el error y agregar en str los que son cadenas como las instrucciones WRITE o INIT_PROC
}instruction;

typedef struct{
    pid_t pid;
    char* fullpath;
    char* filename;
    /// @brief instruction
    t_list* instructions;
}pseudocode;

typedef struct{
    instr_code instrs;
    char* name;
    char* tag;
    char* name2;
    char* tag2;
    int v;
}line_instr;

//TODO: Se podrá cambiar el nombre del pseudocode a query_interpreter

/// @brief Es la estructura que usa el Master para administrar las query
typedef struct{
    int fd;
    qid id;
    state_process sp;
    int pc;
    char* archive_query;
    int priority;
    long start_tick;
    t_temporal* temp;
    long end_tick;
}query;

typedef struct{
    wid id;
    qid id_query;
    int fd;
    int is_free;
    int pc;
}worker;

/// @brief Estructuras para manejo de memoria principal
typedef struct{
    char* file_tag;
    int marco;
    int pag;
    bool modificada; //clock-m
    bool uso; //clock-m

}entrada_tabla_pags;

typedef struct{
    int pag;
    char* file_tag;
}dto_file_tag_pag;

typedef struct{
    bool libre;
    void* inicio;
}marco;


#endif