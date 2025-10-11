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
            log_info(logger, "## Query %d: Acción LEER - Dirección Física: %d - Valor Leído %s", actual_worker->id_query, atoi(right), left);
        }else{
            add_string_to_packet(pack,right);
            log_info(logger, "## Query %d: Acción ESCRIBIR - Dirección Física: %d - Valor Escrito %s", actual_worker->id_query, atoi(middle), right);
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
    log_info(logger, "## Query: %d: - Instrucción realizada: %s", actual_worker->id_query, instr);
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
        ejecutar_truncate(parametro1, atoi(parametro2));

        /*t_packet* paq = create_packet();
        add_int_to_packet(paq, CREATE_FILE);
        add_string_to_packet(paq, file);
        add_string_to_packet(paq, tag);
        add_int_to_packet(paq, tam);
        send_and_free_packet(paq, sock_storage);*/
    }
    else if(caso == WRITE)
    {
        ejecutar_write(parametro1, atoi(parametro2), parametro3);
        //log_info(logger, "## Query %d: Acción ESCRIBIR - Dirección Física: %d - Valor Escrito %s", actual_worker->id_query, dir_base, contenido);
    }
    else if(caso == READ)
    {
        char* file = strtok(parametro1, ":");
        char* tag = strtok(NULL, "");
        int dir_base = atoi(parametro2);
        int tamanio = atoi(parametro3);
        ejecutar_read(parametro1, dir_base, tamanio);
        log_info(logger, "## Query %d: Acción Leído - Dirección Física: %d - VALOR NOT IMPLEMENTED", actual_worker->id_query, dir_base);
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
        ejecutar_flush(parametro1); 
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
    else if(caso==NOOP){
        ejecutar_noop();
    }
    else if(caso == END)
    {
        t_packet* paq = create_packet();
        add_int_to_packet(paq, QUERY_END);
        send_and_free_packet(paq, sock_master); 
    }
    log_info(logger, "## Query: %d: - Instrucción realizada: %s", actual_worker->id_query, instr_to_string(caso));
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
    //NOTE: Se podría crear un campo en actual_worker para el PROGRAM_COUNTER
    log_info(logger, "## Query: %d: -FETCH - Program Counter: %d - %s", actual_worker->id_query, actual_worker->pc, instruccion);
    ejecutar_instruccion(caso, parametro1, parametro2, parametro3);
}

// FASE DECODE //

// FASE FETCH //

t_list *obtener_instrucciones(char *archivo)
{
    log_orange(logger,"Path Queries: %s, Archivo: %s", cw.path_queries, archivo);
    // la carpeta esta el cw.path_scripts
    char *path_archivo = strcat(cw.path_queries, archivo);
    log_orange(logger,"CONCAT: %s", path_archivo);
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



t_list* obtener_instrucciones_v2(char* fullpath){
    
    FILE* f = fopen(fullpath, "r");
    if(f == NULL){
        log_error(logger, "No se pudo abrir el archivo en %s:%d", __func__, __LINE__);
        return NULL;
    }    
    char* line = NULL;
    size_t len=0;
    ssize_t read;
    t_list* res = list_create();
    while((read = getline(&line, &len, f)) != -1){
        if(string_is_empty(line))
            break;
        
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        char* cop = malloc(strlen(line)+1);
        strcpy(cop, line);
        list_add(res, cop);
        if(feof(f))
            break;
    }
    fclose(f);
    if(line)
        free(line);

    return res;
}


// FASE FETCH //

void loop_atender_queries()
{

    for (;;) // 1 iteracionn por query atendida
    {
        log_pink(logger, "Esperando una nueva Query...");
        sem_wait(&sem_query_recibida); 
        log_pink(logger, "Recibi una QUERYYYYYYYYYYYYYYYYYYYYYY");
        //log_pink(logger, "Esperando una nueva Query...");
        //actual_query->instructions = obtener_instrucciones(archivo_query_actual);
        actual_query->instructions = obtener_instrucciones_v2(string_from_format("%s%s", cw.path_queries, archivo_query_actual));
        //t_list *instrucciones = obtener_instrucciones(archivo_query_actual);
        actual_worker->is_free = false;
        
        while (!actual_worker->is_free) // 1 iteracion por instruccion
        {
            //Incrementá el PC negro y con chequeo de out-bound si tenés 10 instrucciones no te podés ir a la instrucción 11 porque se hace percha.
            // Fase Fetch
            
            if(actual_worker->pc >= list_size(actual_query->instructions) || need_stop)
            {
                log_debug(logger, "Voy a mandar un QUERY_END porque el pc actual: %d supero a la instrs size: %d o porque need_stop es true: %d", actual_worker->pc, list_size(actual_query->instructions), need_stop);
                //QUERY END Termina esto que te rre fuiste
                actual_worker->is_free = true;
                t_packet* p = create_packet();
                add_int_to_packet(p, QUERY_END);
                add_int_to_packet(p, actual_worker->id_query);
                add_int_to_packet(p, actual_worker->pc);
                send_and_free_packet(p, actual_worker->fd);
                
                //free_query(actual_query);
                
                break;
            }
            char *instruccion = list_get(actual_query->instructions, actual_worker->pc); //Nótese que incrementa el pc
            log_debug(logger, "QID=%d, PC=%d, Instrucción que va a ejecutar: %s", 
                actual_worker->id_query, 
                actual_worker->pc,
                instruccion
            );
            //log_info(logger, "## Query: %d: -FETCH - Program Counter: %d - %s", actual_worker->id_query, actual_worker->pc, instruccion);
            decode_y_execute(instruccion);
            actual_worker->pc++;
        }

        actual_worker->is_free=true;
    }
}

#endif