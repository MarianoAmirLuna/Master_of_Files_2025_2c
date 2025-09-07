#include "main.h"

int main(int argc, char* argv[]) {
    //TODO: ESTE MODULO TAMBIEN TIENE EL superblock.config y el blocks_hash_index.config
    itself_ocm = MODULE_STORAGE;
    load_config("storage.config");
    cs =load_config_storage();
    create_log("storage", cs.log_level);
    log_violet(logger, "%s", "Hola soy STORAGE");

    inicializar_file_system();

    int sock_server = server_connection(cs.puerto_escucha);

    void* param = malloc(sizeof(int)*2);
    memcpy(param, &sock_server, sizeof(int));
    memcpy(param+sizeof(int), &cs.puerto_escucha, sizeof(int));
    attend_multiple_clients(param);

    return 0;
}


void* attend_multiple_clients(void* params)
{
    int sock=0, port=0;
    memcpy(&sock, params, sizeof(int));
    memcpy(&port, params+sizeof(int), sizeof(int));
    log_debug(logger, "Sock: %d, Port %d", sock,port);
    free(params);
    if(sock <= 0) {
        log_error(logger, "Socket inválido en attend_multiple_clients");
        return NULL;
    }

    for(;;){
        //pthread_mutex_lock(&lock);
        log_debug(logger, "Esperando cliente en socket %d, Puerto: %d", sock, port);
        int sock_client = wait_client(sock, port);
        log_debug(logger, "Socket cliente: %d", sock_client);

        if(handshake(sock_client, 1) != 0)
        {
            log_error(logger, "No pudo hacer handshake con socket %d error %d [%s:%d]", sock_client, errno, __func__, __LINE__);
            close(sock_client);
            continue;  // Continuamos esperando más conexiones en lugar de retornar
        }

        t_list* l = recv_operation_packet(sock_client);
        op_code_module ocm;
        memcpy(&ocm, list_get(l, 0), sizeof(op_code_module));
        list_destroy_and_destroy_elements(l, free_element);



        void* parameter = malloc(sizeof(int)*3);
        int offset = 0;
        memcpy(parameter, &sock_client, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &ocm, sizeof(int));
        offset+=sizeof(int);
        memcpy(parameter+offset, &sock, sizeof(int));
        pthread_t* pth = (pthread_t*)malloc(sizeof(pthread_t));
        pthread_create(pth, NULL,go_loop_net, parameter);
        pthread_detach(*pth);
    }
}

void* go_loop_net(void* params){
    int len=sizeof(int)*3;
    void* data = malloc(len);
    memcpy(data, params, len);
    int sock_client = 0;
    memcpy(&sock_client, data, sizeof(int));
    free(params);
    loop_network(
        sock_client,
        packet_callback,
        data, 
        disconnect_callback
    );
    free(data);
    
    return NULL;
}

// nucleo del storage
void packet_callback(void* params){
    log_info(logger, "Inside here");

    int sock_client = 0;
    op_code_module ocm= 0; // aca veo el modulo que se comunico, deberia ser siempre worker (MODULE_WORKER: 9)
    int sock_server=0;
    int offset= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));
    log_info(logger, "En el packet callback sock_client: %d, ocm: %d, sock_server:%d", sock_client, ocm, sock_server);
    t_list* pack = recv_packet(sock_client); // aca estarian las operaciones

    tratar_mensaje(pack, sock_client); // aca

    if(pack == NULL) {
        log_error(logger, "Error recibiendo paquete");
        return;
    }
    log_pink(logger, "RECIBI DATOS DEL %s", ocm_to_string(ocm));
    // aca es el trabajo del modulo ?
    list_destroy_and_destroy_elements(pack, free_element);
}

void disconnect_callback(void* params){
    int sock_client = 0;
    op_code_module ocm= 0;
    int sock_server=0;
    int offset= 0;
    memcpy(&sock_client, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&ocm, params+offset, sizeof(int));
    offset+=sizeof(int);
    memcpy(&sock_server, params+offset, sizeof(int));

    log_warning(logger, "Se desconectó el cliente %s fd:%d", ocm_to_string(ocm), sock_client);
}










// inicializacion del modulo

void inicializar_file_system()
{

    if (cs.fresh_start) // si es 1 (true)
    {
        log_pink(logger, "no limpio el storage"); //to_do: borrar
    }
    else // si es 0 (false)
    {
        log_pink(logger, "limpio el storage"); //to_do: borrar
        limpiar_fs();
    }
}




void limpiar_fs() {
    eliminar_contenido(cs.punto_montaje);
}

void eliminar_contenido(const char* path) {
    DIR* dir = opendir(path);
    struct dirent* entry;

    if (dir == NULL) {
        perror("No se pudo abrir el directorio");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar "." y ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Ignorar el archivo superblock.config
        if (strcmp(entry->d_name, "superblock.config") == 0)
            continue;

        // Construir path completo
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("No se pudo obtener información del archivo");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Es un directorio: llamar recursivamente y luego eliminarlo
            eliminar_contenido(full_path);
            if (rmdir(full_path) != 0)
                perror("Error al eliminar directorio");
        } else {
            // Es un archivo
            if (remove(full_path) != 0)
                perror("Error al eliminar archivo");
        }
    }

    closedir(dir);
}



/*
Inicialización
Al momento de inicializar el FS lo primero que se debe verificar es el valor FRESH_START del archivo de configuración.
Este valor nos indicará si se debe formatear el volumen (iniciando un FS nuevo) o si se quiere mantener el contenido preexistente.

Al inicializar desde cero el FS (FRESH_START=TRUE), el único archivo nativo que necesitamos tener obligatoriamente es el archivo superblock.config.
Ese archivo nos indicará los datos necesarios para poder formatear nuestro FS,
eliminando previamente los demás archivos que componen nuestro FS en caso de existir.

También se deberá crear un primer File llamado "initial_file" confirmado con un único Tag "BASE",
cuyo contenido será un (1) bloque lógico con el bloque físico nro 0 (cero) asignado, completando el bloque con el caracter 0,
por ejemplo: "00000…". Dicho File/Tag no se podrá borrar.
*/


// fin inicializacion del modulo




















// decodificacion de mensaje y su derivacion para su tratamiento

tratar_mensaje(pack, sock_client)
{
    storage_operation so = list_get_int(pack, 0); 
    
} 



/*

void ejecutar_instruccion(instr_code caso, char *parametro1, char *parametro2)
{
    switch (caso)
    {
    case READ:
        acceder_a_espacio_usuario(0, parametro1, parametro2);
        break;
    case WRITE:
        acceder_a_espacio_usuario(1, parametro1, parametro2);
        break;
    case INIT_PROC:
        init_proc(parametro1, parametro2);
        break;
    case GOTO:
        go_to(parametro1);
        break;
    case IO:
        io(parametro1, parametro2); // En el enunciado es un parametro, pero a ellos no les sube el agua al tanque
        break;
    case NOOP:
        noop();
        break;
    case DUMP_MEMORY:
        dump_memory();
        break;
    case INSTR_SYSCALL_EXIT:
        salir();
        break;
    case INVALID_INSTRUCTION:
        log_info(cpu_logger, "Matate y grabalo ludoBo");
        break;
    }
}

    instr_code cast_code(instruccion)

*/

// fin decodificacion de mensaje y su derivacion para su tratamiento











