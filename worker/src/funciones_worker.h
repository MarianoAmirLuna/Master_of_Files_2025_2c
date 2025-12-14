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
        // El tamaño del archivo debe ser 0
        ejecutar_create(parametro1);
    }
    else if (caso == TRUNCATE)
    {
        ejecutar_truncate(parametro1, atoi(parametro2));
    }
    else if (caso == WRITE)
    {

        int len = strlen(parametro3);
        int base = atoi(parametro2);

        for (int offset = 0; offset < len; offset += block_size)
        {

            // Si el fragmento actual excede el tamaño total del contenido (len),
            // se ajusta para que solo tome los bytes restantes.
            int slice_len = block_size;
            if (offset + slice_len > len)
            {
                slice_len = len - offset; // Último pedazo si no completa el block_size
            }

            char buffer[block_size + 1];
            memcpy(buffer, parametro3 + offset, slice_len);
            buffer[slice_len] = '\0'; // SOLO para que ejecutar_write reciba un string válido

            ejecutar_write(parametro1, base + offset, buffer);
        }
    }
    else if (caso == READ)
    {
        int base = atoi(parametro2);
        int len = atoi(parametro3);

        // Crear un buffer acumulador para almacenar el contenido leído
        char *contenido_total = malloc(len + 1); // Reservar memoria para todo el contenido
        contenido_total[0] = '\0';               // Inicializar el string acumulador como vacío

        for (int offset = 0; offset < len; offset += block_size)
        {
            // Si el fragmento actual excede el tamaño total a leer (len),
            // se ajusta para que solo tome los bytes restantes.
            int slice_len = block_size;
            if (offset + slice_len > len)
            {
                slice_len = len - offset; // Último pedazo si no completa el block_size
            }

            // Leer la página actual
            char *paginaLeida = ejecutar_read(parametro1, base + offset, slice_len);

            // Concatenar el contenido leído al acumulador
            strcat(contenido_total, paginaLeida);

            // Liberar la memoria de la página leída
            free(paginaLeida);
        }

        mandarLecturaAMaster(contenido_total, parametro1);
        free(contenido_total);
    }
    else if (caso == TAG)
    {
        t_packet *paq = create_packet();
        add_int_to_packet(paq, TAG_FILE);
        add_file_tag_to_packet(paq, parametro1);
        add_file_tag_to_packet(paq, parametro2);
        send_and_free_packet(paq, sock_storage);
    }
    else if (caso == COMMIT)
    {
        char **copia_ft = string_split(parametro1, ":");
        ejecutar_flush(parametro1, true);
        ejecutar_commit(copia_ft[0], copia_ft[1]);
        string_array_destroy(copia_ft);
        log_pink(logger, "ESTOY ESPERANDO EL FINFLUSH");
        sem_wait(&fin_de_flush);
        log_pink(logger, "TERMINE DE ESPERAR ESTOY ESPERANDO EL FINFLUSH");
    }
    else if (caso == FLUSH)
    {
        ejecutar_flush(parametro1, true);
        sem_wait(&fin_de_flush);
        log_pink(logger, "ESTOY ESPERANDO EL FINFLUSH 1");
        log_pink(logger, "TERMINE DE ESPERAR ESTOY ESPERANDO EL FINFLUSH 1");
    }
    else if (caso == DELETE)
    {
        t_packet *paq = create_packet();
        add_int_to_packet(paq, DELETE_TAG);
        add_file_tag_to_packet(paq, parametro1);
        send_and_free_packet(paq, sock_storage);
    }
    else if (caso == END)
    {
        t_packet *paq = create_packet();
        add_int_to_packet(paq, QUERY_END);
        add_int_to_packet(paq, actual_query->id);
        add_int_to_packet(paq, actual_query->pc);
        send_and_free_packet(paq, sock_master);
        if (need_desalojo)
        {
            sem_post(&sem_need_desalojo);
            need_desalojo = 0;
        }
        actual_worker->is_free=1;
    }
    log_info(logger, "## Query: %d: - Instrucción realizada: %s", actual_query->id, instr_to_string(caso));
    log_debug(logger, "Query: %d (PC=%d): Valor de actual_workerfree=: %d, Need_desalojo=%d", actual_query->id, actual_query->pc, actual_worker->is_free, need_desalojo);
    
}

void decode_y_execute(char *linea_de_instruccion)
{

    char *instruccion = NULL, *parametro1 = NULL, *parametro2 = NULL, *parametro3 = NULL;
    remove_new_line(linea_de_instruccion);
    char **spl = string_split(linea_de_instruccion, " ");
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
    log_pink(logger, "VOY A ESPERAR LA RE PUTA RESPUESTA");
    sem_wait(&sem_de_esperar_la_puta_respuesta);
    log_pink(logger, "[NO SAQUEN ESTA MIERDA HASTA QUE ESTEN SEGURO DE QUE FUNCIONE TODO] TERMINE DE ESPERAR LA PUTA RESPUESTA");
}

// FASE DECODE //

// FASE FETCH //
t_list *obtener_instrucciones_v2(char *fullpath)
{

    FILE *f = fopen(fullpath, "r");
    if (f == NULL)
    {
        log_error(logger, "No se pudo abrir el archivo en %s:%d", __func__, __LINE__);
        return NULL;
    }
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    t_list *res = list_create();
    while ((read = getline(&line, &len, f)) != -1)
    {
        if (string_is_empty(line))
            break;

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
        {
            line[len - 1] = '\0';
        }
        char *cop = malloc(strlen(line) + 1);
        strcpy(cop, line);
        list_add(res, cop);
        if (feof(f))
            break;
    }
    fclose(f);
    if (line)
        free(line);
    return res;
}

int check_need_desalojo(){
    if (!need_desalojo)
        return 0;

    sem_post(&sem_need_desalojo);
    need_desalojo = 0;
    return 1;
}

// FASE FETCH //
void loop_atender_queries()
{
    for (;;) // 1 iteracionn por query atendida
    {
        if (hubo_error)
        {
            log_pink(logger, "ESTOY ESPERANDO EN SEM_DIMI");
            sem_wait(&sem_dimi);
            log_pink(logger, "TERMINE DE ESPERAR ESPERANDO EN SEM_DIMI");
        }
        //log_trace(logger, "Esperando una nueva Query...");
        check_need_desalojo();
        if(!hubo_query)
        {
            msleep(25);
            continue;
        }
        //sem_wait(&sem_query_recibida);
        log_trace(logger, "Recibi una QUERYYYYYYYYYYYYYYYYYYYYYY");
        char *fullpath = string_from_format("%s%s", cw.path_queries, archivo_query_actual);
        actual_query->instructions = obtener_instrucciones_v2(fullpath);
        int sz = list_size(actual_query->instructions);
        log_trace(logger, "CANTIDAD DE INSTRUCCIONES QUE TIENE EL QUERY PATH: %s ES %d", archivo_query_actual, sz);
        free(fullpath);
        actual_worker->is_free = false;
        log_debug(logger, "Estoy en el ciclo infinito fuera del while_actual_worker");
        while (!actual_worker->is_free) //Tiene que ser una disyunción enfermo... o cumple uno o cumple el otro no podés una conjunción
        {
            if(hubo_error){
                break;
            }
            log_debug(logger, "Estoy en el ciclo");
            // Incrementá el PC negro y con chequeo de out-bound si tenés 10 instrucciones no te podés ir a la instrucción 11 porque se hace percha.
            //  Fase Fetch
            if (check_need_desalojo()){
                break;
            }
            if (need_stop)
            {
                check_need_desalojo();
                log_debug(logger, "Voy a mandar un QUERY_END porque solicitaron un desalojo | need_stop es true: %d", need_stop);
                // QUERY END Termina esto que te rre fuiste
                actual_worker->is_free = true;
                t_packet *p = create_packet();
                add_int_to_packet(p, QUERY_END);
                add_int_to_packet(p, actual_query->id);
                add_int_to_packet(p, actual_query->pc);
                send_and_free_packet(p, sock_master);
                free_query(actual_query);
                break;
            }
            if (actual_query == NULL)
            {
                log_error(logger, "ACTUAL QUERY ES NULO %s:%d", __func__, __LINE__);
                break;
            }
            if (actual_query->instructions == NULL)
            {
                log_error(logger, "ACTUAL QUERY INSTRUCTIONS ES NULO %s:%d", __func__, __LINE__);
                break;
            }
            if (list_size(actual_query->instructions) == 0)
            {
                log_error(logger, "ACTUAL QUERY INSTRUCTIONS NO TIENE ELEMENTOS %s:%d", __func__, __LINE__);
                break;
            }
            if (actual_query->pc >= list_size(actual_query->instructions))
            {
                log_error(logger, "TE FUISTE A LA MIERDA CON EL PC %s:%d", __func__, __LINE__);
                break;
            }
            log_light_blue(logger, "Query ID=%d PC=%d", actual_query->id, actual_query->pc);
            char *instruccion = list_get(actual_query->instructions, actual_query->pc); // Nótese que incrementa el pc
            log_debug(logger, "QID=%d, PC=%d, Instrucción que va a ejecutar: %s",
                      actual_query->id,
                      actual_query->pc,
                      instruccion);
            log_pink(logger, "Estoy por ejecutar el: %s", instruccion);
            decode_y_execute(instruccion);
            //log_pink(logger, "SE TERMINO LA EJECUCION DE: %s", instruccion);
            actual_query->pc++;
        }
        if (hubo_error)
        {
            log_error(logger, "ERROR, se salio de la ejecucion del query por un error de storage");
            need_desalojo = 1;
            log_light_green(logger, "need_desalojo seteada a 1 por error de storage");
        
        }
        log_debug(logger, "Estoy fuera del ciclo actual_worker is free");
        check_need_desalojo();
        hubo_query=false;
    }
}

#endif