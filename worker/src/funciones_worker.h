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

/*void ejecutar_instruccion_v2(instr_code caso, char* instr){
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
}*/

// FASE EXECUTE //
void ejecutar_instruccion(instr_code caso, char *parametro1, char *parametro2, char *parametro3)
{
    log_debug(logger, "EJECUTAR_INSTRUCCION %s | %s | %s", parametro1, parametro2, parametro3);
    if (caso == CREATE)
    {
        //El tamaño del archivo debe ser 0
        ejecutar_create(parametro1);
    }
    else if(caso==TRUNCATE)
    {
        ejecutar_truncate(parametro1, atoi(parametro2));
    }
    else if(caso == WRITE)
    {
        ejecutar_write(parametro1, atoi(parametro2), parametro3);
    }
    else if(caso == READ)
    {
        int dir_base = atoi(parametro2);
        int tamanio = atoi(parametro3);
        ejecutar_read(parametro1, dir_base, tamanio);
    }
    else if(caso == TAG)
    {
        t_packet* paq = create_packet();
        add_int_to_packet(paq, TAG_FILE);
        add_file_tag_to_packet(paq, parametro1);
        add_file_tag_to_packet(paq, parametro2);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==COMMIT)
    {
        ejecutar_flush(parametro1, true); 
        t_packet* paq = create_packet();
        add_int_to_packet(paq, COMMIT_TAG);
        add_file_tag_to_packet(paq, parametro1);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==FLUSH)
    {
        ejecutar_flush(parametro1, true); 
    }
    else if(caso==DELETE)
    {
        t_packet* paq = create_packet();
        add_int_to_packet(paq, DELETE_TAG);
        add_file_tag_to_packet(paq, parametro1);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==NOOP){ 
        //El NOOP no existe en este TP
        ejecutar_noop();
    }
    else if(caso == END)
    {
        t_packet* paq = create_packet();
        add_int_to_packet(paq, QUERY_END);
        add_int_to_packet(paq, actual_query->id);
        add_int_to_packet(paq, actual_query->pc);
        send_and_free_packet(paq, sock_master); 
        actual_worker->is_free=true;
    }
    log_info(logger, "## Query: %d: - Instrucción realizada: %s", actual_query->id, instr_to_string(caso));
}

void decode_y_execute(char *linea_de_instruccion)
{

    char *instruccion = NULL, *parametro1 = NULL, *parametro2 = NULL, *parametro3 = NULL;
    remove_new_line(linea_de_instruccion);

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
    log_info(logger, "## Query: %d: -FETCH - Program Counter: %d - %s", actual_query->id, actual_query->pc, instruccion);
    ejecutar_instruccion(caso, parametro1, parametro2, parametro3);
}

// FASE DECODE //

// FASE FETCH //
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
        log_trace(logger, "Esperando una nueva Query...");
        sem_wait(&sem_query_recibida); 
        log_trace(logger, "Recibi una QUERYYYYYYYYYYYYYYYYYYYYYY");
        char* fullpath = string_from_format("%s%s", cw.path_queries, archivo_query_actual);
        actual_query->instructions = obtener_instrucciones_v2(fullpath);
        int sz = list_size(actual_query->instructions);
        log_trace(logger, "CANTIDAD DE INSTRUCCIONES QUE TIENE EL QUERY PATH: %s ES %d", archivo_query_actual, sz);
        free(fullpath);
        actual_worker->is_free = false;

        while (!actual_worker->is_free)
        {
            //Incrementá el PC negro y con chequeo de out-bound si tenés 10 instrucciones no te podés ir a la instrucción 11 porque se hace percha.
            // Fase Fetch
            
            if(need_stop)
            {
                log_debug(logger, "Voy a mandar un QUERY_END porque solicitaron un desalojo | need_stop es true: %d", need_stop);
                //QUERY END Termina esto que te rre fuiste
                actual_worker->is_free = true;
                t_packet* p = create_packet();
                add_int_to_packet(p, QUERY_END);
                add_int_to_packet(p, actual_query->id);
                add_int_to_packet(p, actual_query->pc);
                send_and_free_packet(p, sock_master);
                
                break;
            }
            if(actual_query == NULL){
                log_error(logger, "ACTUAL QUERY ES NULO %s:%d", __func__, __LINE__);
                break;
            }
            if(actual_query->instructions == NULL)
            {
                log_error(logger, "ACTUAL QUERY INSTRUCTIONS ES NULO %s:%d", __func__, __LINE__);
                break;
            }
            if(list_size(actual_query->instructions) == 0){
                log_error(logger, "ACTUAL QUERY INSTRUCTIONS NO TIENE ELEMENTOS %s:%d", __func__, __LINE__);
                break;
            }
            char *instruccion = list_get(actual_query->instructions, actual_query->pc); //Nótese que incrementa el pc
            log_debug(logger, "QID=%d, PC=%d, Instrucción que va a ejecutar: %s", 
                actual_query->id, 
                actual_query->pc,
                instruccion
            );
            decode_y_execute(instruccion);
            actual_query->pc++;
        }
    }
}

#endif