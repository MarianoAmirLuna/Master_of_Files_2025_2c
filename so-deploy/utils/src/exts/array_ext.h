#ifndef EXTS_ARRAY_EXT
#define EXTS_ARRAY_EXT

#ifndef _STDARG_H
#include <stdarg.h>
#include <stdio.h>
#endif

/// [DEPRECATED]
/// @brief Dynamic variadic arguments 
/// @param n Cantidad de elementos
/// @param sz tamaño del tipo; ejemplo sizeof(char*), sizeof(int), etc
/// @param ... Todo el conjunto igual
/// @return Array de argumentos
/// @example
/// void** v =variadic_pointer(2, sizeof(char*), (void*)"pepe jugó al doom", (void*)"juan");
/// for(int i=0;i<2;i++)
///     printf("[%d]:%s\n", i, (char*)(v[i]));
/*void* variadic_pointer(int n, size_t sz, ...)
{   
    void* res = malloc(n*sz);
    va_list ptr;
    size_t l = n;
    va_start(ptr, l);
    for (int i = 0; i < n; i++){
        void* data = va_arg(ptr, void*);
        memcpy(res+(i*sz), &data, sz);
    }
    va_end(ptr);
    return res;
}*/

/*void** variadic_pointer(int n, size_t sz, ...)
{   
    void** res = malloc(n*sz);
    va_list ptr;
    size_t l = n;
    //__builtin_va_start(ptr, size);
    va_start(ptr, l);
    for (int i = 0; i < n; i++)
        res[i] = va_arg(ptr, void*);
    va_end(ptr);
    return res;
}*/

/// @brief Variadic de Enteros
/// @param n Cantidad de elementos
/// @param  ... Todos los números
/// @return puntero array
int* variadic_int(int n, ...)
{
    int* res = malloc(n*sizeof(int));
    va_list ptr;
    va_start(ptr, n);
    for (int i = 0; i < n; i++)
        res[i] =va_arg(ptr, int); 
    va_end(ptr);
    return res;
}

//WARNING: UNSTABLE BAD COUNTER SOME TIME
int length_vector(void** v)
{
    int res =0;
    int idx=0;
    if(v == NULL)
        return 0;
    for(;;)
    {
        if(v[idx++] == NULL)
            break;
        res++;
    }
    return res;
}
#endif