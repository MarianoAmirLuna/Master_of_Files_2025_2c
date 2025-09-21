#ifndef FUNCIONES_WORKER_H
#define FUNCIONES_WORKER_H

#ifndef INICIALIZAR_WORKER_H
#include "inicializar_worker.h"
#endif

#ifndef WORKER_BASE_H
#include "base.h"
#endif

#ifndef EXTS_STRING_H
#include "exts/string_ext.h"
#endif

void ejecutar_instruccion_v2(instr_code caso, char* instr){
    t_packet* pack = create_packet();
    add_int_to_packet(pack, caso);
    
    if(caso == CREATE || caso == FLUSH || caso == COMMIT || caso == DELETE) //estos casos tienen siempre "[file:tag]"
    {
        
        add_file_tag_to_packet(pack, instr);
        send_and_free_packet(pack, sock_storage);
        return;
    }
    if(caso == TRUNCATE) //este caso tiene siempre "[file:tag] [número]"
    {
        char* left = string_new();
        char* right = string_new();
        get_space_instr(instr, left, right);

        add_file_tag_to_packet(pack, left);
        add_int_to_packet(pack, atoi(right));
        send_and_free_packet(pack, sock_storage);
        free(left);
        free(right);
        return;
    }
    if(caso == READ || caso == WRITE) //este caso tiene siempre "[file:tag] [número] [<número (si es READ) | cadena (si es WRITE)>]"
    {
        char* left = string_new();
        char* middle = string_new();
        char* right = string_new();
        get_two_space_instr(instr, left, middle, right);
        add_file_tag_to_packet(pack, left);
        add_int_to_packet(pack,atoi(middle));
        if(caso == READ){
            add_int_to_packet(pack,atoi(right));
        }else{
            add_string_to_packet(pack,right);
        }

        send_and_free_packet(pack, sock_storage);
        free(left);
        free(middle);
        free(right);
        return;
    }
    if(caso == END) //este caso no tiene una goma
    {
        send_and_free_packet(pack, sock_storage);
    }

}

// FASE EXECUTE //
void ejecutar_instruccion(instr_code caso, char *parametro1, char *parametro2, char *parametro3)
{
    log_debug(logger, "%s | %s | %s", parametro1, parametro2, parametro3);
    if (caso == CREATE)
    {
        //El tamaño del archivo debe ser 0
        char *file = strtok(parametro1, ":");
        char *tag  = strtok(NULL, "");

        t_packet* paq = create_packet();
        add_int_to_packet(paq, CREATE_FILE);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==TRUNCATE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int tam = atoi(parametro2);

        ejecutar_truncate(strcat(file, tag), tam);

        t_packet* paq = create_packet();
        add_int_to_packet(paq, CREATE_FILE);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        add_int_to_packet(paq, tam);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso == WRITE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int dir_base = atoi(parametro2);
        char* contenido = parametro3;
        ejecutar_write(parametro1, dir_base, contenido);
    }
    else if(caso == READ)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int dir_base = atoi(parametro2);
        int tamanio = atoi(parametro3);
        ejecutar_read(parametro1, dir_base, tamanio);
    }
    else if(caso == TAG)
    {
        char* file_old = strtok(parametro1, ":");
        char* tag_old = strtok(NULL, "");

        char* file_new = strtok(parametro2, ":");
        char* tag_new = strtok(NULL, "");

        t_packet* paq = create_packet();
        add_int_to_packet(paq, TAG_FILE);
        add_string_to_packet(paq, file_old);
        add_string_to_packet(paq, tag_old);
        add_string_to_packet(paq, file_new);
        add_string_to_packet(paq, tag_new);
        send_and_free_packet(paq, sock_storage);
        
    }
    else if(caso==COMMIT)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");

        //paquete
        t_packet* paq = create_packet();
        add_int_to_packet(paq, CREATE_FILE);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==FLUSH)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");

        //paquete
        t_packet* paq = create_packet();
        add_int_to_packet(paq, CREATE_FILE);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==DELETE)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");

        //paquete
        t_packet* paq = create_packet();
        add_int_to_packet(paq, DELETE_TAG);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso == END)
    {
        t_packet* paq = create_packet();
        add_int_to_packet(paq, QUERY_END);
        send_and_free_packet(paq, sock_master); 
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

void quitar_salto_linea(char *linea)
{
    size_t len = strlen(linea);
    if (len > 0 && linea[len - 1] == '\n')
    {
        linea[len - 1] = '\0';
    }
}
void decode_y_execute_v2(char *linea_de_instruccion)
{
    char* instr = string_new();
    char* case_str = string_new();
    get_space_instr(linea_de_instruccion, instr, case_str);
    instr_code caso = cast_code(case_str);
    ejecutar_instruccion_v2(caso, instr);
    free(instr);
    free(case_str);
}

void decode_y_execute(char *linea_de_instruccion)
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

t_list* obtener_instrucciones_v2(char* archivo){
    char* fullpath = string_from_format("%s/%s", cw.path_queries, archivo);
    if(!file_exists(fullpath)){
        log_error(logger, "El archivo \"%s\" no existe", fullpath);
        return NULL;
    }
    FILE* f = fopen(fullpath, "r");
    if(f == NULL){
        log_error(logger, "No se pudo abrir el archivo en %s:%d", __func__, __LINE__);
        return NULL;
    }    
    t_list* instrs = list_create();
    char* line = NULL;
    size_t len=0;
    ssize_t read;
    
    while((read = getline(&line, &len, f)) != -1){
        if(string_is_empty(line))
            break;
        list_add(instrs, string_to_buffer(line));
        if(feof(f))
            break;
    }
    fclose(f);
    if(line)
        free(line);
    return instrs;
}

t_list *obtener_instrucciones(char *archivo)
{
    // la carpeta esta el cw.path_scripts
    char *path_archivo = strcat(cw.path_queries, archivo);
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
        sem_wait(&sem_query_recibida); //TODO: no se hace post

        t_list *instrucciones = obtener_instrucciones(archivo_query_actual);
        is_free = false;

        while (!is_free) // 1 iteracion por instruccion
        {
            // Fase Fetch
            char *instruccion = list_get(instrucciones, pc_actual);

            decode_y_execute(instruccion);
        }
    }
}

#endif