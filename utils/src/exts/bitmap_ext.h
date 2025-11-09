#ifndef BITMAP_EXT_H
#define BITMAP_EXT_H

#ifndef INC_LIBS_H
#include "inc/libs.h"
#endif 
#include <sys/mman.h>
#include "commons/bitarray.h"

void ocupar_bloque(t_bitarray* g_bitmap, off_t nro_bloque, size_t g_bitmap_size) {
    bitarray_set_bit(g_bitmap, nro_bloque);
    msync(g_bitmap->bitarray, g_bitmap_size, MS_SYNC);
    log_debug(logger, "Bloque %d marcado como OCUPADO", nro_bloque);
}

void liberar_bloque(t_bitarray* g_bitmap, off_t nro_bloque, size_t g_bitmap_size) {
    bitarray_clean_bit(g_bitmap, nro_bloque);
    msync(g_bitmap->bitarray, g_bitmap_size, MS_SYNC);
    log_debug(logger, "Bloque %d marcado como LIBRE", nro_bloque);
}

bool bloque_ocupado(t_bitarray* g_bitmap, int nro_bloque) {
    return bitarray_test_bit(g_bitmap, nro_bloque);
}

void destruir_bitmap(t_bitarray* g_bitmap, int g_bitmap_fd) {
    bitarray_destroy(g_bitmap);
    close(g_bitmap_fd);
}
#endif