#ifndef FASE_EXECUTE_H
#define FASE_EXECUTE_H

#ifndef MEMORIA_H
#include "memoria.h"
#endif

void ejecutar_create(char* file, char* tag)
{

}

void ejecutar_truncate(char* file_y_tag, int tam)
{

}

void ejecutar_write(char* file, char* tag, int dir_base, char* contenido)
{
    //Chequear si en archivos_cargados existe strcat(file, tag)
    /*  
        si no existe => creamos una nueva entrada que sera una tabla de paginas
        y se asocia la entrada de la tabla de paginas con una entrada de lista_frames
    */
   //  si existe, se evalua el agregar una nueva pagina a la tabla
}

void ejecutar_read(char* file, char* tag, int dir_base, int tam)
{

}

void ejecutar_tag(char* file_old, char* tag_old, char* file_new, char* tag_new)
{

}

void ejecutar_commit(char* file, char* tag)
{

}

void ejecutar_flush(char* file, char* tag)
{

}

void ejecutar_delete(char* file, char* tag)
{

}

void ejecutar_end()
{
    
}

#endif