#ifndef FUNCIONES_WORKER_H
#define FUNCIONES_WORKER_H

//#include "base.h"
int pc_actual;
config_worker cw;

char* archivo_query_actual;

#include "inicializar_worker.h"
#include "fase_execute.h"

// FASE EXECUTE //

void ejecutar_instruccion(instr_code caso, char *parametro1, char *parametro2, char *parametro3)
{
    log_debug(logger, "%s | %s | %s", parametro1, parametro2, parametro3);
    if (caso == CREATE)
    {
        char *file = strtok(parametro1, ":");
        char *tag  = strtok(NULL, "");
    }
    else if(caso==TRUNCATE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int tam = atoi(parametro2);
    }
    else if(caso == WRITE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int dir_base = atoi(parametro2);
        char* contenido = parametro3;
    }
    else if(caso == READ)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int dir_base = atoi(parametro2);
        int tamanio = atoi(parametro3);
    }
    else if(caso == TAG)
    {
        char* file_old = strtok(parametro1, ":");
        char* tag_old = strtok(NULL, "");

        char* file_new = strtok(parametro2, ":");
        char* tag_new = strtok(NULL, "");
    }
    else if(caso==COMMIT)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
    }
    else if(caso==FLUSH)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
    }
    else if(caso==DELETE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
    }
    else if(caso == END)
    {
        //??????????????????????    
    }
}

/*
CREATE <NOMBRE_FILE>:<TAG>
TRUNCATE <NOMBRE_FILE>:<TAG> <TAMAÑO>
WRITE <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
READ <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
TAG <NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
COMMIT <NOMBRE_FILE>:<TAG>
FLUSH <NOMBRE_FILE>:<TAG>
DELETE <NOMBRE_FILE>:<TAG>
END
*/

// FASE EXECUTE //

// FASE DECODE //
//[DEPRECATED] USE instr_code cast_code(char*) INSTEAD
/*instr_code str_to_enum(char *instruccion)
{
    if (strcmp(instruccion, "CREATE") == 0)
        return CREATE;
    if (strcmp(instruccion, "TRUNCATE") == 0)
        return TRUNCATE;
    if (strcmp(instruccion, "WRITE") == 0)
        return WRITE;
    if (strcmp(instruccion, "READ") == 0)
        return READ;
    if (strcmp(instruccion, "TAG") == 0)
        return TAG;
    if (strcmp(instruccion, "COMMIT") == 0)
        return COMMIT;
    if (strcmp(instruccion, "FLUSH") == 0)
        return FLUSH;
    if (strcmp(instruccion, "DELETE") == 0)
        return DELETE;
    if (strcmp(instruccion, "END") == 0)
        return END;
    return INVALID_INSTRUCTION;
}*/

void quitar_salto_linea(char *linea)
{
    size_t len = strlen(linea);
    if (len > 0 && linea[len - 1] == '\n')
    {
        linea[len - 1] = '\0';
    }
}

void iniciar_decode(char *linea_de_instruccion)
{

    char *instruccion = NULL, *parametro1 = NULL, *parametro2 = NULL, *parametro3 = NULL;
    quitar_salto_linea(linea_de_instruccion);

    char *token = strtok(linea_de_instruccion, " "); // agarra desde el inicio hasta el primer espacio
    instruccion = token;

    token = strtok(NULL, " ");
    if (token != NULL)
    {
        parametro1 = token; // agarra desde el primer espacio hasta el segundo
    }

    token = strtok(NULL, " ");
    if (token != NULL)
    {
        parametro2 = token; // agarra desde el segundo espacio hasta el fin
    }

    token = strtok(NULL, " ");
    if (token != NULL)
    {
        parametro3 = token; // agarra desde el tercer espacio hasta el fin
    }

    instr_code caso = cast_code(instruccion);

    ejecutar_instruccion(caso, parametro1, parametro2, parametro3);
}

// FASE DECODE //

// FASE FETCH //

t_list *obtener_instrucciones(char *archivo)
{
    // la carpeta esta el cw.path_scripts
    char *path_archivo = strcat(cw.path_scripts, archivo);
    FILE *lector_de_archivo; // Declarar un puntero a FILE
    char *linea;             // linea leida del archivo

    lector_de_archivo = fopen(path_archivo, "r"); // Abrir el archivo en modo lectura

    if (lector_de_archivo == NULL)
    {
        printf("No se pudo abrir el lector_de_archivo.\n");
        fclose(lector_de_archivo); // Cerrar el archivo en caso de error
    }

    // aqui se puede trabajar con el lector_de_archivo
    t_list *instrucciones_por_pc = list_create();

    while (fgets(linea, sizeof(linea), lector_de_archivo) != NULL)
    {
        // TO DO: quitar esta linea despues de las pruebas de lectura de archivo
        printf("%s", linea); // Imprimir cada linea leida
        list_add(instrucciones_por_pc, linea);
        // si lee bien la linea deberia ir guardandolo en la lista
    }

    fclose(lector_de_archivo); // Cerrar el archivo

    return instrucciones_por_pc;
}

// FASE FETCH //

void loop_atender_queries()
{

    for (;;) // 1 iteracionn por query atendida
    {
        sem_wait(&sem_query_recibida);

        t_list *instrucciones = obtener_instrucciones(archivo_query_actual);
        is_free = false;

        while (!is_free) // 1 iteracion por instruccion
        {
            // Fase Fetch
            char *instruccion = list_get(instrucciones, pc_actual);

            iniciar_decode(instruccion);
        }
    }
}

#endif