#ifndef TEST_STORAGE_H
#define TEST_STORAGE_H
#include "base.h"

// agrega un int a la lista
static inline void add_int(t_list* list, int value) {
    int* v = malloc(sizeof(int));
    *v = value;
    list_add(list, v);
}

// agrega un string a la lista
static inline void add_string(t_list* list, const char* value) {
    list_add(list, strdup(value));
}

// wrapper para ejecutar un test
static inline void run_test(const char* name, t_list* pack) {
    log_info(logger, ">>> Ejecutando test: %s", name);
    tratar_mensaje(pack, 1);  // sock_client fake
    list_destroy_and_destroy_elements(pack, free);
}

// ---------- Tests concretos ----------

static inline void test_create_file() {
    t_list* pack = list_create();
    add_int(pack, CREATE_FILE);
    add_string(pack, "test_file tag_0001");
    run_test("CREATE_FILE test_file tag_0001", pack);
}

static inline void test_truncate_file() {
    t_list* pack = list_create();
    add_int(pack, TRUNCATE_FILE);
    add_string(pack, "test_file tag_0001 512");
    run_test("TRUNCATE_FILE test_file tag_0001 512", pack);
}

static inline void test_tag_file() {
    t_list* pack = list_create();
    add_int(pack, TAG_FILE);
    add_string(pack, "test_file tag_0001 TAGCOPIA algo");
    run_test("TAG_FILE test_file tag_0001 TAGCOPIA algo", pack);
}

static inline void test_commit_tag() {
    t_list* pack = list_create();
    add_int(pack, COMMIT_TAG);
    add_string(pack, "test_file tag_0001");
    run_test("COMMIT_TAG test_file tag_0001", pack);
}

static inline void test_write_block() {
    t_list* pack = list_create();
    add_int(pack, WRITE_BLOCK);
    add_string(pack, "test_file tag_0001 0 hola_mundo");
    run_test("WRITE_BLOCK test_file tag_0001 0 hola_mundo", pack);
}

static inline void test_read_block() {
    t_list* pack = list_create();
    add_int(pack, READ_BLOCK);
    add_string(pack, "test_file tag_0001 0");
    run_test("READ_BLOCK test_file tag_0001 0", pack);
}

static inline void test_delete_tag() {
    t_list* pack = list_create();
    add_int(pack, DELETE_TAG);
    add_string(pack, "test_file tag_0001");
    run_test("DELETE_TAG test_file tag_0001", pack);
}

// ---------- todos los casos al hilo ----------

static inline void run_all_tests() {
    test_create_file();
    test_truncate_file();
    test_tag_file();
    test_commit_tag();
    test_write_block();
    test_read_block();
    test_delete_tag();
}

#endif
