#include "base.h"


void crear_directorio(char* nombre, char* path)
{
    char dir[1024];

    snprintf(dir, sizeof(dir), "%s/%s", path, nombre);

    if(mkdir(dir, 0777) == 0) 
    {
        log_pink(logger, "Se creo el directorio %s, en el path %s", nombre, path); //to_do: borrar este
    }

}


bool control_existencia(const char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL)
    {
        return false;
    }
    struct stat statbuf;
    if (stat(path, &statbuf)==0)
    {
        //Path encontrado
        return true;
    }
    else 
    {
        //Path no encontrado
        return false;
    }
}


bool control_existencia_file(const char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL)
    {
        return false;
    }
    log_warning(logger, "path: %s", path);
    FILE * archivo = fopen(path, "r");
    if (archivo != NULL)
    {
    //Path encontrado
        log_info(logger, "existe el archivo");
        fclose(archivo);
        return true;
    }
    else 
    {
        log_error(logger, "archivo no encontrado");
        //Path no encontrado
        return false;
    }
}

int cant_elementos_directorio (const char *path)
{
    int cant_elementos = 0;
    DIR * dir = opendir(path);

    if (!dir)
    {
        log_error(logger, "No se pudo abrir el directorio %s", path);
        return -1;
    }

    struct dirent * entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (!string_equals_ignore_case(entry->d_name, ".") && !string_equals_ignore_case(entry->d_name, ".."))
        {
            cant_elementos++;
        }
    }

    closedir(dir);
    return cant_elementos;
}

void crear_archivo (const char* path, const char* nombre, const char* extension)
{
    char* p_path = string_from_format("%s/%s.%s", path, nombre, extension);
    if (!control_existencia(p_path))
    {
        FILE * archivo = fopen(p_path, "w");
        fclose(archivo);
    }
    else
    {
        log_warning(logger, "El archivo %s.%s ya existe en el path %s", nombre, extension, path);
    }
    free(p_path);
}
/*
void eliminar_archivo (const char* path, const char* nombre)
{
    char* p_path = string_from_format("%s/%s", path, nombre);
    if (control_existencia(p_path))
    {
        FILE * archivo = fopen(p_path, "r");
        remove(archivo);
    }
    else
    {
        log_error(logger, "El archivo %s no existe en el path %s", nombre, path);
    }
    free(p_path);
}
*/
void eliminar_archivo (const char* path, const char* nombre)
{
    char* p_path = string_from_format("%s/%s", path, nombre);
    if (control_existencia(p_path)) {
        remove(p_path);   // ✅ recibe const char*
    } else {
        log_error(logger, "El archivo %s no existe en el path %s", nombre, path);
    }
    free(p_path);
}



void crear_hard_link(char* path_bloque_fisico, char* path_bloque_logico)
{
    if(link(path_bloque_fisico, path_bloque_logico) == 0)
    {
        log_debug(logger, "se vinculo el bloque del path %s, con el bloque en el path %s", path_bloque_logico, path_bloque_fisico);
    }
    else
    {
        perror("link");
        log_error(logger, "ERROR EN VINCULACION DE BLOQUES LOGICO: %s, FISICO: %s", path_bloque_logico, path_bloque_fisico);
    }
}

void crear_config_path(char* p_path, char* name)
{
    char* path = string_from_format("%s/%s", p_path, name);
    create_blocks_hash_index(path);
    free(path);
}

t_config* crear_metadata_config(char* path, int tamanio, t_list* bloques, state_metadata estado) {
    // path debe terminar en /metadata.config
    t_config* metadata = config_create(path);
    if (metadata == NULL) {
        metadata = malloc(sizeof(t_config));
        metadata->path = strdup(path);
        metadata->properties = dictionary_create();
    }

    // TAMAÑO
    config_set_value(metadata, "TAMAÑO", string_itoa(tamanio));

    // BLOCKS → construir string "[0,1,2,...]"
    char* bloques_str;
    if (list_is_empty(bloques)) {
        bloques_str = strdup("[]");
    } else {
        char* tmp = string_new();
        string_append(&tmp, "[");
        for (int i = 0; i < list_size(bloques); i++) {
            int* b = list_get(bloques, i);
            char* num = string_itoa(*b);
            string_append(&tmp, num);
            free(num);
            if (i < list_size(bloques) - 1) string_append(&tmp, ",");
        }
        string_append(&tmp, "]");
        bloques_str = tmp;
    }
    config_set_value(metadata, "BLOCKS", bloques_str);
    free(bloques_str);

    // ESTADO
    const char* estado_str = (estado == WORK_IN_PROGRESS) ? "WORK_IN_PROGRESS" : "COMMITED";
    config_set_value(metadata, "ESTADO", (char*)estado_str);

    config_save(metadata);
    return metadata;
}

void llenar_archivo_con_ceros(char* path_archivo) 
{
    FILE* archivo = fopen(path_archivo, "wb");
    if (!archivo) {
        log_error(logger, "No se pudo abrir el archivo %s para llenarlo", path_archivo);
        return;
    }

    // buffer temporal con '0'
    char* buffer = malloc(g_block_size);
    if (!buffer) {
        log_error(logger, "No se pudo reservar memoria para llenar %s", path_archivo);
        fclose(archivo);
        return;
    }

    memset(buffer, '0', g_block_size);  // llenar con el caracter '0'
    fwrite(buffer, 1, g_block_size, archivo);

    free(buffer);
    fclose(archivo);
}




void eliminar_bloques_fisicos (int cantidad_bloques_de_mas, char* path, int bloques_actuales)
{
    // Elimino los bloques fisicos que sobren
    char* p_path;
    for (int i = 0; i < cantidad_bloques_de_mas; i++)
    {
        p_path = string_from_format("block%04d.dat", bloques_actuales-i);
        eliminar_archivo(path, p_path);
        free(p_path);
    }
}




/*
void crear_bloques_fisicos (int cantidad_bloques_faltantes, char* path, int bloques_actuales)
{
    // Creo los bloques fisicos que falten
    for (int i = 0; i < cantidad_bloques_faltantes; i++)
    {
        crear_archivo(path, string_from_format("block%04d", bloques_actuales+i), "dat");
        llenar_archivo_con_ceros(string_from_format("%s/%s.%s", path, "block%04d", bloques_actuales+i, "dat"));
    }
}
*/
void crear_bloques_fisicos (int cantidad_bloques_faltantes, char* path, int bloques_actuales)
{
    for (int i = 0; i < cantidad_bloques_faltantes; i++)
    {
        char* nombre = string_from_format("block%04d", bloques_actuales + i);

        crear_archivo(path, nombre, "dat");

        char* full_path = string_from_format("%s/%s.dat", path, nombre);
        llenar_archivo_con_ceros(full_path);

        free(nombre);
        free(full_path);
    }
}