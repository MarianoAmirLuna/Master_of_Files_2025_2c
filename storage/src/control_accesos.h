#ifndef CONTROL_ACCESOS_H
#define CONTROL_ACCESOS_H

#ifndef STORAGE_BASE_H
#include "base.h"
#endif 
t_dictionary* file_tag_locks;
pthread_mutex_t file_tag_locks_lock;
pthread_mutex_t* block_locks;
pthread_mutex_t bitmap_lock;

void creo_semaforos_fs()
{
    file_tag_locks = dictionary_create();
    pthread_mutex_init(&file_tag_locks_lock, NULL);
    pthread_mutex_init(&bitmap_lock, NULL);
}


pthread_mutex_t* get_file_tag_lock(char* file, char* tag) {
    char* key = string_from_format("%s:%s", file, tag);

    pthread_mutex_lock(&file_tag_locks_lock);

    pthread_mutex_t* lock = dictionary_get(file_tag_locks, key);
    if (lock == NULL) {
        lock = malloc(sizeof(pthread_mutex_t));
        pthread_mutex_init(lock, NULL);
        dictionary_put(file_tag_locks, key, lock);
    } else {
        free(key);
    }

    pthread_mutex_unlock(&file_tag_locks_lock);

    return lock;
}



/*
void destruir_file_tag_locks() {
    void destroy_lock(char* key, void* lock) {
        pthread_mutex_destroy((pthread_mutex_t*) lock);
        free(lock);
    }
    dictionary_destroy_and_destroy_elements(file_tag_locks, destroy_lock);
}
*/



void init_block_locks() {
    int cantidad_bloques = g_fs_size / g_block_size;
    block_locks = malloc(sizeof(pthread_mutex_t) * cantidad_bloques);
    for (int i = 0; i < cantidad_bloques; i++) {
        pthread_mutex_init(&block_locks[i], NULL);
    }
}


#endif