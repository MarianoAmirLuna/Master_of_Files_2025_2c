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

// FASE EXECUTE //
void ejecutar_instruccion(instr_code caso, char *parametro1, char *parametro2, char *parametro3)
{
    log_debug(logger, "EJECUTAR_INSTRUCCION: (%s) | %s | %s | %s", instr_to_string(caso), parametro1, parametro2, parametro3);
    if (caso == CREATE)
    {
        log_pink(logger,"Aca 1");

        //El tamaño del archivo debe ser 0
        ejecutar_create(parametro1);
        sem_wait(&sem_respuesta_storage_success);
    }
    else if(caso==TRUNCATE)
    {
        log_pink(logger,"Aca 2");

        ejecutar_truncate(parametro1, atoi(parametro2));
        log_pink(logger,"Aca 3");
        sem_wait(&sem_respuesta_storage_success);
    }
    else if(caso == WRITE)
    {
        log_pink(logger,"Aca 4");

        ejecutar_write(parametro1, atoi(parametro2), parametro3);
    }
    else if(caso == READ)
    {
        log_pink(logger,"Aca 5");
        int dir_base = atoi(parametro2);
        int tamanio = atoi(parametro3);
        ejecutar_read(parametro1, dir_base, tamanio);
    }
    else if(caso == TAG)
    {
        log_pink(logger,"Aca 6");
        t_packet* paq = create_packet();
        add_int_to_packet(paq, TAG_FILE);
        add_file_tag_to_packet(paq, parametro1);
        add_file_tag_to_packet(paq, parametro2);
        send_and_free_packet(paq, sock_storage);
        sem_wait(&sem_respuesta_storage_success);
    }
    else if(caso==COMMIT)
    {
        log_pink(logger,"Aca 7");
        ejecutar_flush(parametro1, true); 
        t_packet* paq = create_packet();
        add_int_to_packet(paq, COMMIT_TAG);
        add_file_tag_to_packet(paq, parametro1);
        send_and_free_packet(paq, sock_storage);
        sem_wait(&sem_respuesta_storage_success);
    }
    else if(caso==FLUSH)
    {
        log_pink(logger,"Aca 8");
        ejecutar_flush(parametro1, true);  
    }
    else if(caso==DELETE)
    {
        log_pink(logger,"Aca 9");
        t_packet* paq = create_packet();
        add_int_to_packet(paq, DELETE_TAG);
        add_file_tag_to_packet(paq, parametro1);
        send_and_free_packet(paq, sock_storage);
    }
    else if(caso==NOOP){ 
        log_pink(logger,"Aca 10");
        //El NOOP no existe en este TP
        ejecutar_noop();
    }
    else if(caso == END)
    {
        log_pink(logger,"Aca 11");
        t_packet* paq = create_packet();
        add_int_to_packet(paq, QUERY_END);
        add_int_to_packet(paq, actual_query->id);
        add_int_to_packet(paq, actual_query->pc);
        send_and_free_packet(paq, sock_master); 
        if(need_desalojo){
            sem_post(&sem_need_desalojo);
            need_desalojo=0;
        }
        actual_worker->is_free=true;
    }
    log_info(logger, "## Query: %d: - Instrucción realizada: %s", actual_query->id, instr_to_string(caso));
}

void decode_y_execute(char *linea_de_instruccion)
{

    char *instruccion = NULL, *parametro1 = NULL, *parametro2 = NULL, *parametro3 = NULL;
    remove_new_line(linea_de_instruccion);
    char** spl = string_split(linea_de_instruccion, " ");
    instruccion = spl[0];
    int sz = string_array_size(spl);
    
    if (sz > 1 && spl[1] != NULL && !string_is_empty(spl[1]))
        parametro1 = spl[1];
    if (sz > 2 && spl[2] != NULL && !string_is_empty(spl[2]))
        parametro2 = spl[2];
    if (sz > 3 && spl[3] != NULL && !string_is_empty(spl[3]))
        parametro3 = spl[3];
    
    instr_code caso = cast_code(instruccion);
    log_info(logger, "## Query: %d: -FETCH - Program Counter: %d - %s", actual_query->id, actual_query->pc, instruccion);
    ejecutar_instruccion(caso, parametro1, parametro2, parametro3);
    string_array_destroy(spl);
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
        if(need_desalojo){
            sem_post(&sem_need_desalojo);
            need_desalojo=0;
        }
        sem_wait(&sem_query_recibida); 
        log_trace(logger, "Recibi una QUERYYYYYYYYYYYYYYYYYYYYYY");
        char* fullpath = string_from_format("%s%s", cw.path_queries, archivo_query_actual);
        actual_query->instructions = obtener_instrucciones_v2(fullpath);
        int sz = list_size(actual_query->instructions);
        log_trace(logger, "CANTIDAD DE INSTRUCCIONES QUE TIENE EL QUERY PATH: %s ES %d", archivo_query_actual, sz);
        free(fullpath);
        actual_worker->is_free = false;
        log_warning(logger, "Estoy en el ciclo infinito fuera del while_actual_worker");
        while (!actual_worker->is_free && !hubo_error)
        {
            log_warning(logger, "Estoy en el ciclo");
            //Incrementá el PC negro y con chequeo de out-bound si tenés 10 instrucciones no te podés ir a la instrucción 11 porque se hace percha.
            // Fase Fetch
            if(need_desalojo){
                actual_worker->is_free=true;
                sem_post(&sem_need_desalojo);
                need_desalojo=0;
                break;
            }
            if(need_stop)
            {
                if(need_desalojo){
                    actual_worker->is_free=true;
                    sem_post(&sem_need_desalojo);
                    need_desalojo=0;
                    //break;
                }
                log_debug(logger, "Voy a mandar un QUERY_END porque solicitaron un desalojo | need_stop es true: %d", need_stop);
                //QUERY END Termina esto que te rre fuiste
                actual_worker->is_free = true;
                t_packet* p = create_packet();
                add_int_to_packet(p, QUERY_END);
                add_int_to_packet(p, actual_query->id);
                add_int_to_packet(p, actual_query->pc);
                send_and_free_packet(p, sock_master);
                free_query(actual_query);
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
            log_light_blue(logger, "Query ID=%d PC=%d", actual_query->id, actual_query->pc);
            char *instruccion = list_get(actual_query->instructions, actual_query->pc); //Nótese que incrementa el pc
            log_debug(logger, "QID=%d, PC=%d, Instrucción que va a ejecutar: %s", 
                actual_query->id, 
                actual_query->pc,
                instruccion
            );
            log_pink(logger, "Estoy por ejecutar el: %s", instruccion);
            decode_y_execute(instruccion);
            log_pink(logger, "SE TERMINO LA EJECUCION DE: %s", instruccion);
            actual_query->pc++;
        }
        if(hubo_error){
            log_error(logger, "ERROR, se salio de la ejecucion del query por un error de storage");
            need_desalojo=1;
            log_light_green(logger, "need_desalojo seteada a 1 por error de storage");
        }
        log_warning(logger, "Estoy fuera del ciclo actual_worker is free");
        if(need_desalojo){
            actual_worker->is_free=true;
            sem_post(&sem_need_desalojo);
            need_desalojo=0;
        }
    }
}

#endif