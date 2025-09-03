#ifndef MODULES_THREADS_MANAGER_H
#define MODULES_THREADS_MANAGER_H

#include "pthread.h"
#include "commons/collections/dictionary.h"
#include "commons/string.h"
#ifndef LIBS_LOGGER_H
#include "../../libs/logger.h"
#endif

typedef struct tpth
{
    int threads_idx;
    pthread_t* pth;
}threads;

t_dictionary* threads_dicts;
int unique_threads_idx=0;



threads* add_thread_by_name(char* name)
{
    if(threads_dicts == NULL)
        threads_dicts = dictionary_create();
    if(dictionary_has_key(threads_dicts, name)){
        log_info(logger, "Ya existe este thread %s", name);
        return NULL;
    }
    log_trace(logger, "Added thread name %s", name);
    threads* th = malloc(sizeof(threads));
    th->pth = (pthread_t*)malloc(sizeof(pthread_t));
    th->threads_idx = unique_threads_idx;
    
    dictionary_put(threads_dicts, name, th);
    unique_threads_idx+=1;
    //free(name);
    return th;
}

threads* add_thread_by_int(int name){
    
    char* nn = string_from_format("%d", name);
    threads* aux = add_thread_by_name(nn);
    free(nn);
    return aux;
}
threads* add_thread_by_enum(int enumer){
    return add_thread_by_int(enumer);
}
threads* add_thread_by_name_with_mtx(char* name)
{
    threads* th = add_thread_by_name(name);
    return th;
}

threads* add_thread()
{
    threads* th = add_thread_by_int(unique_threads_idx);
    return th;
}

threads* get_threads_by_name(char* name){
    return (threads*)dictionary_get(threads_dicts, name);
}
threads* get_threads_by_int(int name){
    char* f = string_itoa(name);
    threads* th = (threads*)dictionary_get(threads_dicts, f);
    free(f);
    return th;
}
threads* get_threads_by_enum(int enumer){
    return get_threads_by_int(enumer);
}
pthread_t* get_pthread_by_enum(int enumer){
    return get_threads_by_int(enumer)->pth;
}
pthread_t* get_pthread_by_name(char* name){
    return get_threads_by_name(name)->pth;
}


#endif