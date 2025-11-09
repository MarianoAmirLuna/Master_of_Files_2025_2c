#ifndef COMMUNICATION_OPS_DELETE_TAG_H
#define COMMUNICATION_OPS_DELETE_TAG_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"
#endif
#ifndef BITMAP_EXT_H
#include "exts/bitmap_ext.h"
#endif

void delete_tag_ops(char* file, char* tag, worker* w){
    /*Esta operación eliminará el directorio correspondiente al File:Tag indicado. 
    Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado 
    no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.*/
    // Para la implementación de esta parte se recomienda consultar la documentación de la syscall stat(2).

    /* EXCEPCIONES A TENER EN CUENTA EN ESTE PROCEDIMIENTO
        File_inexistente
        Tag_inexistente
    */
    //- Borrar directorio
    //- Si el bloque físico al que apunta cada bloque lógico eliminado no es referenciado por ningún otro File:Tag deberá ser marcado como libre en bitmap

    int nblock = 0;
    //WARNING: Debe obtenerse el número de bloque para especificar cuál liberar.
    liberar_bloque(g_bitmap, nblock, g_bitmap_size); // to_do: eliminar esta linea una vez implementado
    //char* logical = get_logical_blocks_dir(cs, file, tag);
    char* fullpathdir = string_from_format("%s/%s/%s", cs.punto_montaje, file, tag);
    if(!control_existencia(fullpathdir)){
        /*t_packet* p = create_packet();
        add_int_to_packet(p, );
        send_and_free_packet(p, w->fd);*/
        log_error(logger, "No se encontro directorio");
        free(fullpathdir);
        return;
    }

    delete_directory(fullpathdir);
    free(fullpathdir);
    log_error(logger, "%s NOT IMPLEMENTED (%s:%d)",__func__, __func__,__LINE__);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}

#endif