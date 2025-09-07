#ifndef EXTS_COMMON_EXT
#define EXTS_COMMON_EXT

#ifndef UTILS_STRUCTS_H
#include "../utils/structs.h"
#endif
#include "sys/stat.h"
#include "time.h"
#include "stdio.h"

#define MIN(x,y) (((x)<(y)) ? (x) : (y))
#define MAX(x,y) (((x)>(y)) ? (x) : (y))

void free_element(void* elem){
    if(elem)
        free(elem);
}

/// @brief Opcode module to string
/// @param ocm Op code module
/// @return a char* of opcode module
char* ocm_to_string(op_code_module ocm){
    if(ocm==MODULE_QUERY_CONTROL){
        return "MODULE_QUERY_CONTROL";
    }
    if(ocm==MODULE_MASTER){
        return "MODULE_MASTER";
    }
    if(ocm==MODULE_WORKER){
        return "MODULE_WORKER";
    }
    if(ocm==MODULE_STORAGE){
        return "MODULE_STORAGE";
    }
    return string_new();
}

char* instr_to_string(instr_code icode){
    if(icode == CREATE){
        return "CREATE";
    }
    if(icode == TRUNCATE){
        return "TRUNCATE";
    }
    if(icode == WRITE){
        return "WRITE";
    }
    if(icode == READ){
        return "READ";
    }
    if(icode == TAG){
        return "TAG";
    }
    if(icode == COMMIT){
        return "COMMIT";
    }
    if(icode == FLUSH){
        return "FLUSH";
    }
    if(icode == DELETE){
        return "DELETE";
    }
    if(icode == END){
        return "END";
    }
    return string_new();
}

char* state_to_string(state_process sp){
    if(sp == STATE_READY){
        return "STATE_READY";
    }
    if(sp == STATE_EXEC){
        return "STATE_EXEC";
    }
    if(sp == STATE_EXIT){
        return "STATE_EXIT";
    }
    return string_new();
}
char* scheduler_mode_to_string(scheduler_mode mode){
    if(mode == SHORT_SCHEDULER)
        return "SHORT_SCHEDULER";
    if(mode == MEDIUM_SCHEDULER)
        return "MEDIUM_SCHEDULER";
    if(mode == LONG_SCHEDULER)
        return "LONG_SCHEDULER";
    return string_new();
}

int file_exists(char* filename){
    struct stat buffer;
    return stat(filename, &buffer) == 0 ? 1 : 0;
}

int directory_exists(char* dir){
    struct stat sb;
    return stat(dir, &sb) == 0 && S_ISDIR(sb.st_mode) ? 1 : 0;
}

int file_create(char* filename){
    FILE* f = fopen(filename, "w+b"); //Quiero crear o abrir el archivo como binario
    return fclose(f);
}

/// @brief WARNING el archivo está abierto. Si no se usa más se debe cerrar
/// @param filename 
/// @return 
FILE* file_create_without_close(char* filename){
    FILE* f = fopen(filename, "w+b"); //Quiero crear o abrir el archivo como binario
    return f;
}

FILE* write_empty_seek(FILE* f, int sz, int seek){
    fseek(f, seek, SEEK_SET);
    char* buffer = (char*)malloc(sz*sizeof(char));
    for(int i=0;i<sz;i++)
        buffer[i] = '\0';
    if(fwrite(buffer, sizeof(char), sz, f) != sz){
        log_error(logger, "No se pudo escribir satisfactoriamente el archivo con tamaño: %d", sz);
        return NULL;
    }
    return f;
}

FILE* write_empty(FILE* f, int sz)
{
    return write_empty_seek(f, sz, 0);
}

/// @brief WARNING el archivo está abierto. Si no se usa más se debe cerrar
/// @param filename 
/// @return 
FILE* file_create_size_without_close(char* filename, int sz){
    FILE* f = fopen(filename, "w+b"); //Quiero crear o abrir el archivo como binario
    return write_empty(f, sz);
}


/*int file_write(char* filename, void* data, int len){
    FILE* f = fopen(filename, "wb+");
    //fwrite()
}*/

/// @brief 
/// @param f 
/// @return this NOT CLOSE THE FILE 
long int file_size(FILE* f){
    fseek(f, 0L, SEEK_END);
    long int res=  ftell(f);
    fseek(f, 0L, SEEK_SET);
    return res;
}

long int file_size_read(char* file){
    FILE* f = fopen(file, "r");
    long int res = file_size(f);
    fclose(f);
    return res;
}

long int file_size_and_close(FILE* f)
{
    long int res= file_size(f);
    fclose(f);
    return res;
}

int file_write(char* filepath, void* data, int len){
    FILE* f = fopen(filepath, "w+b");
    fwrite(data, len, 1, f);
    return fclose(f);
}

/// @brief Duerme el subproceso en tiempo de milisegundos
/// @param msec 
/// @return 
int msleep(long msec){
    struct timespec ts;
    if(msec  < 0){
        return -1;
    }
    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;
    nanosleep(&ts, NULL);
    return EXIT_SUCCESS;
}

#endif