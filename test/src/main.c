#include "main.h"
#include "../../storage/src/funciones_generales.h"
#include "../../storage/src/inicializacion_storage.h"
#include "../../storage/src/control_accesos.h"
#include "../../storage/src/comunicacion_worker.h"
#include "../../storage/src/test.h"



t_list *obtener_instrucciones(char *archivo)
{
    //log_orange(logger,"Path Queries: %s, Archivo: %s", cw.path_queries, archivo);
    // la carpeta esta el cw.path_scripts
    /*char *path_archivo = strcat(cw.path_queries, archivo);
    log_orange(logger,"CONCAT: %s", path_archivo);*/
    FILE *lector_de_archivo; // Declarar un puntero a FILE
    char *linea;             // linea leida del archivo

    lector_de_archivo = fopen(archivo, "r"); // Abrir el archivo en modo lectura

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

int main(int argc, char* argv[]) {
    
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    load_config("../storage/storage.config");
    config_storage cs = load_config_storage();

    log_info(logger, "%d", cs.puerto_escucha);

    t_list* ll = obtener_instrucciones_v2("/home/utnso/tp-2025-2c-Pizza/worker/queries/pseudo1.txt");
    for(int i=0;i<list_size(ll);i++){
        char* line = (char*)list_get(ll, i);
        log_info(logger, "LINE: %s", line);
    }

    return;

/*
    parse_code(CREATE, "MATERIAS:BASE");
    parse_code(TAG, "MATERIAS:BASE MATERIAS:V2");
    parse_code(TRUNCATE, "MATERIAS:BASE 1024");
    parse_code(READ, "MATERIAS:BASE 0 8");
    return 0;
*/

    op_code_module ocm = MODULE_STORAGE;
    int wcl = client_connection("127.0.0.1", cs.puerto_escucha);
    if(handshake(wcl, 0) != 0)
    {
        log_error(logger, "No pudo hacer handshake con el socket %d del modulo %s exit(1) is invoked", wcl, ocm_to_string(ocm));
        exit(EXIT_FAILURE);
    }
    send_john_snow_packet(MODULE_WORKER, wcl);
    t_list* l = recv_operation_packet(wcl);
    int what_operation_is = list_get_int(l, 0);
    int block_size = list_get_int(l, 1);
    log_orange(logger, "TENGO EL BLOCK SIZE DEL STORAGE: %d", block_size);
    list_destroy_and_destroy_elements(l, free_element);



    t_packet* pack = create_packet();
/*
    add_int_to_packet(pack, CREATE_FILE);
    add_string_to_packet(pack, "test_file2 tag_0001");
    send_and_free_packet(pack, wcl);
*/

    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0001 2 aaaa");
    send_and_free_packet(pack, wcl);

/*
    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0005 2 aaaa");
    send_and_free_packet(pack, wcl);
*/


    void* parameters = malloc(sizeof(int)*3);
    int len_args = 2;
    
    int offset = 0;
    memcpy(parameters, &len_args, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &ocm, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &wcl, sizeof(int));

    loop_network(wcl, packet_callback, parameters, NULL);

    return 0;
}


void packet_callback(void* params){
    int cntargs = 0;
    int sock = -1;
    op_code_module ocm=0;

    int offset = 0;
    memcpy(&cntargs, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock, params+offset, sizeof(int));
    //free(params);
    log_debug(logger, "OCM: %s", ocm_to_string(ocm));
    t_list* packet = recv_packet(sock);
    log_info(logger, "Recibi mensaje de %s cantidad del packet: %d", ocm_to_string(ocm), list_size(packet));
    
    int op_code = list_get_int(packet, 0); //si del otro lado se usa el add_int_to_packet(...) entonces es recomendable utilizar el list_get_int(...) si por alguna razón da malos valores intentar con casteo (int)...
    
    if(ocm == MODULE_MASTER){
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE MASTER
        if(op_code == EJECUTAR_QUERY){
            
        }
    }
    if(ocm == MODULE_STORAGE){
        //ACA RECIBIS UN PAQUETE PROVENIENTE DE STORAGE
        if(op_code == BLOCK_SIZE)
        {
            int storage_block_size = list_get_int(packet, 1);
        }
    }
    

    list_destroy_and_destroy_elements(packet, free_element);
}
