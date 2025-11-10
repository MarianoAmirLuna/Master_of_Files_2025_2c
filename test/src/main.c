#include "main.h"
#include "commons/crypto.h"

/*int create_nested_directories(const char *path) {
    char** spl = string_split(path, "/");
    int sz = string_array_size(spl);
    char* build = string_new();
    for(int i=0;i<sz;i++){
        string_append(&build, spl[i]);
        if(i < sz -1){
            string_append(&build, "/");
        }
        if(mkdir(build, 0777) != 0 && errno != EEXIST){
            perror("Error creating directory");
            string_array_destroy(spl);
            free(build);
            return -1;
        }
    }
    string_array_destroy(spl);
    return 1;
}*/

int main(int argc, char* argv[]) {
    
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    load_config("../storage/storage.config");
    config_storage cs = load_config_storage();
    /*char* lero = "HOLA MUNDO";
    void* vava = malloc(strlen(lero)+1);
    memcpy(vava, lero, strlen(lero));
    char* lero1 = "HOLA MUNDO 2";
    void* vava1 = malloc(strlen(lero1)+1);
    memcpy(vava1, lero1, strlen(lero1));*/
    char* bb = get_block_name(4);
    log_debug(logger, "BLOCK NAME: %s", bb);
    bb = get_block_name(171);
    log_debug(logger, "BLOCK NAME: %s", bb);
    bb = get_block_name(3094);
    log_debug(logger, "BLOCK NAME: %s", bb);
    bb = get_block_name_by_n(45914,6);
    log_debug(logger, "BLOCK NAME: %s", bb);

    log_pink(logger, "%s", get_name_fmt_number("block_", 5, 6));
    log_pink(logger, "%s", get_name_extension_fmt_number("copado", "dat", 12, 8));
    create_nested_directories("/home/utnso/tp-2025-2c-Pizza/test/bin/lero/que/onda/pepe");
    //mkdir("/home/utnso/tp-2025-2c-Pizza/test/bin/lero", 0777);
    /*t_config* blo = load_block_hash("bloquetest.config");
    t_list* vv = get_all_value__of_block_hash(blo);
    for(int i=0;i<list_size(vv);i++){
        char* val = list_get(vv, i);
        log_debug(logger, "VALOR %s", val);
    }
    log_info(logger, "AHora solo voy a recibir los KEYS");
    t_list* vv1 = get_all_hash_of_block_hash(blo);
    for(int i=0;i<list_size(vv1);i++){
        char* val = list_get(vv1, i);
        log_debug(logger, "VALOR %s", val);
    }
    log_debug(logger, "Existe el hash '68e4b9551869bce5b170e873f5abe1f7': %d", exists_hash_in_block_hash(blo, "68e4b9551869bce5b170e873f5abe1f7"));

    t_config* metadata = config_create("metadata_test.config"); //como ya existe sólo lo abre.
    t_list* blocks = get_array_blocks_as_list_from_metadata(metadata);
    for(int i=0;i<list_size(blocks);i++){
        //int bn = list_get_int(blocks, i);
        log_debug(logger, "BLOCK %d", (int)list_get(blocks, i));
    }
    remove_block_from_metadata(metadata, 8);
    log_pink(logger, "Removi el bloque 8 a ver ahora");
    blocks = get_array_blocks_as_list_from_metadata(metadata);
    for(int i=0;i<list_size(blocks);i++){
        //int bn = list_get_int(blocks, i);
        log_debug(logger, "BLOCK %d", (int)list_get(blocks, i));
    }*/
    /*insert_hash_block(blo, crypto_md5(vava, strlen(lero)), "block_0001");
    insert_hash_block(blo, crypto_md5(vava1, strlen(lero1)), "block_0002");
    t_list* ll = list_create();
    
    list_add(ll, 2);
    list_add(ll, 8);
    list_add(ll, 1);
    list_add(ll, 15);
    create_metadata("metadata_test.config", 2048, ll, WORK_IN_PROGRESS);*/
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
