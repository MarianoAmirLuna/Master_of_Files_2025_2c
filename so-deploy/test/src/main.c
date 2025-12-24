#include "main.h"
#include "commons/crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include "commons/collections/queue.h"

bool comparer(void* a, void *b){
    return (int)a < (int)b;
}

typedef struct {
    char* file_tag;
    int marco;
    int pag;
    bool modificada; // clock-m
    bool uso;        // clock-m
} entrada_tabla_pags;
entrada_tabla_pags* buscar_victima_clock_modificado(void){
    log_debug(logger, "entre a ejecutar_flush()");
    if (queue_size(tabla_pags_global) == 0) {
        log_error(logger, "CLOCK-M: tabla_pags_global vacía, no hay víctima para seleccionar");
        return NULL;
    }

    /* Primera pasada: Buscar (0,0) */
    for (int i = 0; i < queue_size(tabla_pags_global); i++) {
        entrada_tabla_pags* elemento = (entrada_tabla_pags*) queue_pop(tabla_pags_global);
        if (elemento->uso == false && elemento->modificada == false) {
            return elemento;
        }
        queue_push(tabla_pags_global, elemento);
    }
    
    /* Segunda pasada: Buscar (0,1) y resetear el bit de uso */
    for (int i = 0; i < queue_size(tabla_pags_global); i++) {
        entrada_tabla_pags* elemento = (entrada_tabla_pags*) queue_pop(tabla_pags_global);
        if(elemento == NULL){
            log_error(logger, "ELEMENTO ES NULL ESTO NO DEBERIA PASAR (%s:%d)", __func__, __LINE__);
        }
        if (elemento->uso == false && elemento->modificada == true) {
            /* Encontramos la víctima (0,1). La liberamos y salimos. */
            return elemento;
        }
        
        /* Si no es la víctima, reseteamos su bit de uso. */
        elemento->uso = false;
        
        /* Lo volvemos a poner al final de la cola para la próxima pasada. */
        queue_push(tabla_pags_global, elemento);
    }

    /* OJO: acá faltaría un return en tu código real: */
    /* return buscar_victima_clock_modificado(); */
    buscar_victima_clock_modificado();
}

/* =========================
 *  Helpers para los tests
 * ========================= */

/* Vacía la cola (NO libera las entradas, total el proceso termina) */
void limpiar_cola(void) {
    while (queue_size(tabla_pags_global) > 0) {
        (void)queue_pop(tabla_pags_global);
    }
}

/* Crea una entrada y la pushea en la cola global */
entrada_tabla_pags* nueva_entrada(bool uso, bool modificada,
                                  int marco, int pag,
                                  const char* tag) {
    entrada_tabla_pags* e = (entrada_tabla_pags*)malloc(sizeof(entrada_tabla_pags));
    if (e == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    e->file_tag   = (char*)tag;   /* Para test da igual no copiar el string */
    e->marco      = marco;
    e->pag        = pag;
    e->modificada = modificada;
    e->uso        = uso;
    queue_push(tabla_pags_global, e);
    return e;
}

/* =========================
 *  TESTS
 * ========================= */

/* 1) Cola vacía → debe devolver NULL */
void test_cola_vacia(void) {
    limpiar_cola();

    entrada_tabla_pags* victima = buscar_victima_clock_modificado();

    assert(victima == NULL);
    assert(queue_size(tabla_pags_global) == 0);
}

/* 2) Sólo un elemento (0,0) → lo elige en la primera pasada */
void test_un_solo_00(void) {
    limpiar_cola();
    entrada_tabla_pags* e1 = nueva_entrada(false, false, 0, 0, "A"); /* (uso=0, mod=0) */

    entrada_tabla_pags* victima = buscar_victima_clock_modificado();

    assert(victima == e1);
    assert(queue_size(tabla_pags_global) == 0); /* lo sacó de la cola */
}

/* 3) Hay un (0,0) pero NO es el primer elemento (chequea rotación) */
void test_00_no_es_primer_elemento(void) {
    limpiar_cola();
    entrada_tabla_pags* e1 = nueva_entrada(true,  true,  0, 0, "A"); /* (1,1) */
    entrada_tabla_pags* e2 = nueva_entrada(false, false, 0, 1, "B"); /* (0,0) → víctima */
    entrada_tabla_pags* e3 = nueva_entrada(true,  false, 0, 2, "C"); /* (1,0) */

    entrada_tabla_pags* victima = buscar_victima_clock_modificado();

    assert(victima == e2);                  /* eligió el (0,0) */
    assert(queue_size(tabla_pags_global) == 2); /* quedó A y C adentro */

    /* opcional: verificar que A y C sigan en la cola */
    entrada_tabla_pags* r1 = (entrada_tabla_pags*)queue_pop(tabla_pags_global);
    entrada_tabla_pags* r2 = (entrada_tabla_pags*)queue_pop(tabla_pags_global);
    assert((r1 == e1 && r2 == e3) || (r1 == e3 && r2 == e1));
}

/* 4) No hay (0,0), pero sí un (0,1) → lo elige en la segunda pasada
 *    y resetea el bit de uso de los que va pasando.
 */
void test_01_segunda_pasada_y_reseteo(void) {
    limpiar_cola();
    entrada_tabla_pags* e1 = nueva_entrada(true,  false, 0, 0, "A"); /* (1,0) */
    entrada_tabla_pags* e2 = nueva_entrada(false, true,  0, 1, "B"); /* (0,1) → víctima */
    entrada_tabla_pags* e3 = nueva_entrada(true,  true,  0, 2, "C"); /* (1,1) */

    entrada_tabla_pags* victima = buscar_victima_clock_modificado();

    assert(victima == e2);
    /* e1 debería haber sido visitado en la segunda pasada y reseteado a uso=0 */
    assert(e1->uso == false);
    /* e3 nunca se toca porque la función retorna en e2 */
    assert(e3->uso == true);
    assert(queue_size(tabla_pags_global) == 2); /* quedaron A y C en la cola */
}

// =========================
// main de pruebas
// =========================
int main(int argc, char* argv[]) {

    t_queue* tabla_pags_global = queue_create();

    test_cola_vacia();
    test_un_solo_00();
    test_00_no_es_primer_elemento();
    test_01_segunda_pasada_y_reseteo();

    limpiar_cola();
    queue_destroy(tabla_pags_global);

    printf("Todos los tests de CLOCK-M pasaron OK ✅\n");
    return 0;

    /*
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    load_config("../storage/storage.config");
    config_storage cs = load_config_storage();
    char* data_bloque = malloc(16);
    char* primerdato = "Hola mundo";
    memcpy(data_bloque, primerdato, 16);
    log_orange(logger, "Resultado: %s", data_bloque);
    char* segundodato = "Bueno";
    memcpy(data_bloque, segundodato, 16);
    log_orange(logger, "Resultado: %s", data_bloque);
    

    return;

/*
    parse_code(CREATE, "MATERIAS:BASE");
    parse_code(TAG, "MATERIAS:BASE MATERIAS:V2");
    parse_code(TRUNCATE, "MATERIAS:BASE 1024");
    parse_code(READ, "MATERIAS:BASE 0 8");
    return 0;
*/
/*
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
/*
    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0001 2 aaaa");
    send_and_free_packet(pack, wcl);

/*
    add_int_to_packet(pack, WRITE_BLOCK);
    add_string_to_packet(pack, "test_file2 tag_0005 2 aaaa");
    send_and_free_packet(pack, wcl);
*/

/*
    void* parameters = malloc(sizeof(int)*3);
    int len_args = 2;
    
    int offset = 0;
    memcpy(parameters, &len_args, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &ocm, sizeof(int));
    offset+=sizeof(int);
    memcpy(parameters+offset, &wcl, sizeof(int));

    loop_network(wcl, packet_callback, parameters, NULL);

    return 0;*/
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
