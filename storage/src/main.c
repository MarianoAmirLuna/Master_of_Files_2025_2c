#include "main.h"

//Configuracion File System - Variables globales para simplificar desarrollo de las funciones.



/*
+----------------------------------------------------------------------------------------------+
|                        Inicio de Modulo - Conexion con Otros Modulos                         |
+----------------------------------------------------------------------------------------------+
*/


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

        char* superblockpath = string_from_format("%s/%s", cs.punto_montaje, "superblock.config");
        t_config* c_superblock = config_create(superblockpath);
        int block_size = config_get_int_value(c_superblock,"BLOCK_SIZE");

        t_packet* pack = create_packet();
        add_int_to_packet(pack, block_size);
        send_and_free_packet(pack, sock_client);
        
        config_destroy(c_superblock);

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


/*
+----------------------------------------------------------------------------------------------+
|                        Inicio de Modulo - Conexion con Otros Modulos                         |
+----------------------------------------------------------------------------------------------+
*/






// inicializacion del modulo

void inicializar_file_system()
{
    char* path_superblock_config = string_from_format("%s/%s", cs.punto_montaje, "superblock.config");
    t_config* c_superblock = config_create(path_superblock_config);

    g_block_size = config_get_int_value(c_superblock, "BLOCK_SIZE");
    g_fs_size = config_get_int_value(c_superblock, "FS_SIZE");

    if (cs.fresh_start) // si es 1 (true)
    {
        log_pink(logger, "limpio el storage"); //to_do: borrar
        limpiar_fs();
    }
    else // si es 0 (false)
    {
        log_pink(logger, "no limpio el storage"); //to_do: borrar
    }
    valido_bitmap(cs.punto_montaje);
    valido_hash(cs.punto_montaje);
    valido_bloques_fisicos(cs.punto_montaje);
    valido_inicial_file(cs.punto_montaje);
}

void valido_inicial_file(char* path)
{
    // Controlo que exista cada directorio, y en caso que no exista se creada
    // Hayque integrar aca dentro la creacion del meta.config y el 000000.dat, y en el caso de que exista el directorio veriticar la existencia de dichos archivos.
    if (!control_existencia(string_from_format("%s%s", path, "files")))
    {
        crear_directorio("files", path);
    }
    if (!control_existencia(string_from_format("%s%s", path, "files/BASE")))
    {
        crear_directorio("BASE",string_from_format("%s%s", path, "files/"));
    }
    if (!control_existencia(string_from_format("%s%s", path, "files/BASE/logical_blocks")))
    {
        crear_directorio("logical_blocks",string_from_format("%s%s", path, "files/BASE/"));
    }
}

void valido_bitmap(const char* p_path)
{
    char* path = string_from_format("%s%s", p_path, "bitmap.bin");
    FILE* bitmap = fopen(path, "rb");
    // valido si existe el archivo 
    if(bitmap == NULL)
    {
        log_debug(logger, "No se encontro el archivo bitmap en el path %s", path); // to_do: sacar despues
        bitmap = fopen(path, "wb");
        
        if(bitmap == NULL){
            log_error(logger, "Error raro al crear el bitmap"); // to_do: sacar despues.
        }
        inicializar_bitmap();
    }
}

void inicializar_bitmap() {
    size_t num_blocks = g_fs_size / g_block_size;

    size_t bitmap_size = (size_t) ceil((double) num_blocks / 8);

    char* path = string_from_format("%s%s", cs.punto_montaje, "bitmap.bin");
    FILE* bitmap = fopen(path, "wb");
    if (bitmap == NULL) {
        log_error(logger, "Error al crear el archivo bitmap"); //to_do: sacar despues
        return;
    }

    // crear un bitmap vacío (todos los bits en 0)
    unsigned char* bitmap_data = (unsigned char*) calloc(bitmap_size, sizeof(unsigned char));

    if (bitmap_data == NULL) {
        log_error(logger, "Error al reservar memoria para el bitmap");
        fclose(bitmap);
        return;
    }

    // Escribir el bitmap vacío en el archivo
    size_t bytes_written = fwrite(bitmap_data, sizeof(unsigned char), bitmap_size, bitmap);
    if (bytes_written != bitmap_size) {
        log_error(logger, "Error al escribir el bitmap en el archivo");//to_do: sacar despues
    } else {
        log_debug(logger, "Bitmap creado correctamente", "bitmap.bin");//to_do: sacar despues
    }

    free(bitmap_data);

    fclose(bitmap);
}



// valido que exista el archivo blokcs_hash_index.config, si no existe lo creo en blanco
void valido_hash(const char* p_path)
{
    char* path = string_from_format("%s%s", p_path, "blocks_hash_index.config");
    FILE* bitmap = fopen(path, "r");
    // valido si existe el archivo 
    if(bitmap == NULL)
    {
        log_debug(logger, "No se encontro el archivo bitmap en el path %s", path); // to_do: sacar despues
        bitmap = fopen(path, "w");
        
        if(bitmap == NULL){
            log_error(logger, "Error raro al crear el bitmap"); // to_do: sacar despues.
        }
    }
    else
    {
        fclose(bitmap);
    }
    free(path);
}



bool control_existencia(const char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL)
    {
        return false;
    }
    struct stat statbuf;
    if (stat(path, &statbuf)==0)
    {
        //Path encontrado
        return true;
    }
    else 
    {
        //Path no encontrado
        return false;
    }
}

// valido que exista el directorio de bloques fisicos, y que tenga la cantidad necesaria de bloques
// si falta algun bloque lo creo (funcion crear_bloque_fisico(tamaño bloque, nombre_de_bloque), eliminar_bloque_fisico(numero_bloque))
void valido_bloques_fisicos(const char* path)
{
    //Controlo que exista el directorio de bloques fisicos
    if (control_existencia(string_from_format("%s%s", path, "physical_blocks")))
    {
        log_light_green(logger, "[valido_bloques_fisicos] Directorio encontrado, verificando que el block count sea el correcto"); //to_do: borrar este
        int cantidad_bloques = g_fs_size / g_block_size;
        log_light_blue(logger, "[valido_bloques_fisicos] Cantidad de bloques que deberia haber: %d", cantidad_bloques); //to_do: borrar este
    }
    else
    {
        crear_directorio("physical_blocks", path);
        //crear_bloques_fisicos (cantidad_bloques) para crear los bloques fisicos faltantes
    }
   
}




void crear_directorio(char* nombre, char* path)
{
    char dir[1024];

    snprintf(dir, sizeof(dir), "%s/%s", path, nombre);

    if(mkdir(dir, 0777) == 0) 
    {
        log_pink(logger, "Se creo el directorio %s, en el path %s", nombre, path); //to_do: borrar este
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
        if (string_equals_ignore_case(entry->d_name, ".") || string_equals_ignore_case(entry->d_name, ".."))
            continue;

        // Ignorar el archivo superblock.config
        if (string_equals_ignore_case(entry->d_name, "superblock.config"))
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

void tratar_mensaje(t_list* pack, int sock_client)
{
    storage_operation so = list_get_int(pack, 0); 
    
} 

void ejecutar_storage_instruction(storage_operation so, char* par1, char* par2){
    if(so == CREATE_FILE){
        
    }
    if(so == TRUNCATE_FILE){
        
    }
    if(so == TAG_FILE){
        
    }
    if(so == COMMIT_TAG){
        
    }
    if(so == WRITE_BLOCK){
        
    }
    if(so == READ_BLOCK){
        
    }
    if(so == DELETE_TAG){
        
    }
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











