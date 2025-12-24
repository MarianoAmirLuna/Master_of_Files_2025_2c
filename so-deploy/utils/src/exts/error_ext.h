#ifndef EXTS_ERROR_H
#define EXTS_ERROR_H

#include <errno.h>
#include <stdio.h>

/// @brief Imprime en pantalla el nombre de código de error
/// @param from En dónde ocurrió
/// @return 
static int print_error(char* from)
{
    printf("Error [%s]: %s\n", from, strerror(errno));
    return -1;
}

#endif