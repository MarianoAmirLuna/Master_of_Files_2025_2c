#ifndef COMMUNICATION_OPS_TAG_FILE_H
#define COMMUNICATION_OPS_TAG_FILE_H

#ifndef BASE_COMMUNICATION_OPS_H
#include "base_communication_ops.h"

#endif

void tag_file_ops(char* file, char* tag, worker* w){
    
    //Control de que no se reciban cosas nulas
    if(file == NULL || tag == NULL){
        log_error(logger, "FILE o TAG son nulos");
        return;
    }

    //Controlo que exista el archivo y el tag
    if (!control_existencia(string_from_format("%s/%s", "files/", file))){
        log_error(logger, "El archivo %s no existe", file);
        return;
    }
    else{
        if (!control_existencia(string_from_format("%s/%s/%s", "files/", file, tag))){
            log_error(logger, "El tag %s no existe", tag);
            return;
        }
    }

    //Control de que no se intente comitear un archivo que ya estaba comiteado.
    if (tag_comiteado(file, tag)){
        log_debug(logger, "El archivo %s tag %s se comitio correctamente", file, tag);
        return;
    }
    else{

    }

    //
    
    /*
    1. Verifico que no exista ya dicho archivo con el tag indicado.
        a. En ese caso hay que avisarle a el worker que dicha operacion no se puede realizar porque ya existe el archivo.
    2. En caso de que no exita, hay que crear el archivo nuevo
    3. Una vez creado hay que copiar el metadata del archivo original al nuevo, dejandolo en WORK IN PROGRESS.
    4. Como no se pueden tener dos bloques fisicos iguales, lo que hay que hacer es vincular los bloques actuales a los bloques logicos, tener en cuenta que dicha data esta en el archivo de metadatos.
    5. Completada la operacion avisar a Worker que se completo la operacion.
    
    
    */

    log_error(logger, "%s NOT IMPLEMENTED (%s:%d)",__func__, __func__,__LINE__);
    //Si necesitan decirle algo al worker desde este método se crea el paquet y se envía en w->fd send_and_free()
    //Ejemplo: send_and_free_packet(p, w->fd);
}



#endif