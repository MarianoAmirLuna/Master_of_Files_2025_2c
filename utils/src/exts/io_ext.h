#ifndef IO_EXT_H
#define IO_EXT_H


#include <sys/stat.h>
#include <sys/mman.h>


#ifndef INC_LIBS_H
#include "inc/libs.h"
#endif 
#include "commons/string.h"

//#include "file_ext.h"
#include "bitmap_ext.h"

#define MAX_PATH 255

#ifndef LIST_H_
#include "commons/collections/list.h"
#endif
void crear_directorio(char* nombre, char* path)
{
    char* dir = string_from_format("%s/%s", path, nombre);    
    if(mkdir(dir, 0777) == 0) {
        log_pink(logger, "Se creo el directorio %s, en el path %s", nombre, path); //to_do: borrar este
    }
    free(dir);
}

int create_nested_directories(const char *path) {
    char** spl = string_split(path, "/");
    int sz = string_array_size(spl);
    char* build = string_new();
    for(int i=0;i<sz;i++){
        string_append(&build, spl[i]);
        if(i < sz -1){
            string_append(&build, "/");
        }
        if(mkdir(build, 0777) != 0 && errno != EEXIST){
            perror("Error creating directory");
            string_array_destroy(spl);
            free(build);
            return -1;
        }
    }
    string_array_destroy(spl);
    free(build);
    return 1;
}
void delete_directory(char* fullpathdir){
    DIR* dir = opendir(fullpathdir);
    struct dirent* entry;

    if (dir == NULL) {
        perror("No se pudo abrir el directorio");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar "." y ".."
        if (string_equals_ignore_case(entry->d_name, ".") || string_equals_ignore_case(entry->d_name, ".."))
            continue;

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", fullpathdir, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("No se pudo obtener información del archivo");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Borrar recursivamente el subdirectorio (y su contenido)
            delete_directory(full_path);
        } else {
            // Es un archivo
            if (remove(full_path) != 0)
                perror("Error al eliminar archivo");
        }
    }

    closedir(dir);

    // Ahora que el directorio está vacío, lo borramos también
    if (rmdir(fullpathdir) != 0) {
        perror("Error al eliminar directorio");
    }
}

t_list* get_files_from_dir(char* fullpathdir){
    DIR* dir = opendir(fullpathdir);
    struct dirent* entry;
    t_list* res = list_create();
    if (dir == NULL) {
        perror("No se pudo abrir el directorio");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar "." y ".."
        if (string_equals_ignore_case(entry->d_name, ".") || string_equals_ignore_case(entry->d_name, ".."))
            continue;
        list_add(res, strdup(entry->d_name));
    }
    closedir(dir);
    return res;
}

bool control_existencia_file_old(const char* path)
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

/// @brief Verifica si existe un path en el sistema de archivos
/// @param path
/// @return 
int control_existencia(char* path){
    //Controlo que no se me envie un path vacio
    if (path==NULL){
        return false;
    }
    struct stat statbuf;
    if (stat(path, &statbuf)==0){
        //Path encontrado
        return true;
    }
    else 
    {
        //Path no encontrado
        return false;
    }
}

/// @brief Verifica si existe un archivo en el sistema de archivos
/// @param path 
/// @return 
int control_existencia_file(char* path)
{
    //Controlo que no se me envie un path vacio
    if (path==NULL){
        return 0;
    }
    log_warning(logger, "path: %s", path);
    FILE * archivo = fopen(path, "r");
    if (archivo != NULL)
    {
        //Path encontrado
        log_info(logger, "existe el archivo");
        fclose(archivo);
        return 1;
    }
    else 
    {
        log_error(logger, "archivo no encontrado");
        //Path no encontrado
        return 0;
    }
}

bool tag_comiteado(char* file, char* tag)
{
    char* path = string_from_format("files/%s/%s/metadata.config", file, tag);
    t_config* conf = load_config(path);
    char* estado = config_get_string_value(conf, "ESTADO");
    bool is_commit=string_equals_ignore_case(estado, "COMMITED");
    free(path);
    free(estado);
    config_destroy(conf);
    return is_commit;
}

int cant_elementos_directorio(char* path){
    int cnt = 0;
    DIR* dir = opendir(path);
    if(!dir)
    {
        log_error(logger, "No se pudo abrir el directorio %s", path);
        return -1;
    }
    struct dirent* entry;
    
    while ((entry = readdir(dir)) != NULL){
        if (!string_equals_ignore_case(entry->d_name, ".") && !string_equals_ignore_case(entry->d_name, "..")){
            cnt++;
        }
    }

    closedir(dir);
    return cnt;
}

int crear_archivo(char* path, char* nombre, char *extension){
    char* p_path = string_from_format("%s/%s.%s", path, nombre, extension);
    if(control_existencia_file(p_path)){
        log_warning(logger, "El archivo %s.%s ya existe en el path %s", nombre, extension, path);
        free(p_path);
        return -1;
    }
    FILE * archivo = fopen(p_path, "w");
    fclose(archivo);
    free(p_path);
    return 0;
}

void eliminar_archivo (char* path, char* nombre)
{
    char* p_path = string_from_format("%s/%s", path, nombre);
    if (control_existencia(p_path)) {
        remove(p_path);   // ✅ recibe const char* //El ✅ es 0 es elemento nulo
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

t_config* crear_config_block_hash_index(char* p_path, char* name)
{
    char* path = string_from_format("%s/%s", p_path, name);
    t_config* conf = create_blocks_hash_index(path);

    free(path);
    return conf;
}


t_config* crear_metadata_config(char* path, int tamanio, t_list* bloques, state_metadata estado) {
    // path debe terminar en /metadata.config
    return create_metadata(path, tamanio, bloques, estado);

    /*
    t_config* metadata = config_create(path);
    if (metadata == NULL) {
        metadata = malloc(sizeof(t_config));
        metadata->path = string_duplicate(path);
        metadata->properties = dictionary_create();
    }

    // TAMAÑO
    config_set_value(metadata, "TAMAÑO", string_itoa(tamanio));

    // BLOCKS → construir string "[0,1,2,...]"
    char* bloques_str;
    if (bloques == NULL) {
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
    return metadata;*/
}

void llenar_archivo_con_ceros(char* path_archivo, int g_block_size) 
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
    for (int i = 0; i < cantidad_bloques_de_mas; i++)
    {
        char* p_path = string_from_format("block%04d.dat", bloques_actuales-i);
        eliminar_archivo(path, p_path);
        free(p_path);
    }
}

void crear_bloques_fisicos (int cantidad_bloques_faltantes, char* path, int bloques_actuales, int g_block_size)
{
    for (int i = 0; i < cantidad_bloques_faltantes; i++)
    {
        char* nombre = string_from_format("block%04d", bloques_actuales + i);

        crear_archivo(path, nombre, "dat");

        char* full_path = string_from_format("%s/%s.dat", path, nombre);
        llenar_archivo_con_ceros(full_path, g_block_size);

        free(nombre);
        free(full_path);
    }
}

void escribir_bloque_fisico(int bloque_fisico, char* contenido)
{
    // 1) Retardo de acceso al bloque
    msleep(cs.retardo_acceso_bloque);

    // 2) Armamos path
    char* path = string_from_format("%s/physical_blocks/block%04d.dat", cs.punto_montaje, bloque_fisico);

    // 3) Abrimos el archivo físico
    FILE* f = fopen(path, "r+");
    if (f == NULL) {
        // Si no existe, creamos el archivo desde cero
        f = fopen(path, "w+");
        if (f == NULL) {
            log_error(logger, "[WRITE_BLOCK] No se pudo abrir o crear bloque físico %d (%s)",
                      bloque_fisico, path);
            free(path);
            return;
        }
    }

    // 4) Nos aseguramos de escribir EXACTAMENTE g_block_size bytes
    //    Si el contenido mide menos, se completa con '\0'

    // Limpiamos el archivo (por si hay datos viejos)
    fseek(f, 0, SEEK_SET);

    int len = strlen(contenido);

    // Escribimos el contenido primero
    fwrite(contenido, 1, len, f);

    // Si falta completar hasta g_block_size:
    if (len < g_block_size) {
        int padding = g_block_size - len;
        char* zeros = calloc(1, padding);
        fwrite(zeros, 1, padding, f);
        free(zeros);
    }

    fflush(f);
    fclose(f);
    free(path);
}

/*
void escribir_bloque_fisico(int bloque_fisico, char* contenido)
{
    char* path = string_from_format("%s/physical_blocks/block%04d.dat",cs.punto_montaje, bloque_fisico);

    FILE* f = fopen(path, "r+");

    if( f = NULL )
    {
        log_error(logger, "[WRITE_BLOCK] no se pudo abrir el bloque fisico %d en %s", bloque_fisico, path);
        free(path);
        return;
    }
}
*/
/**
 * Busca el primer bloque físico libre en el bitmap y clona el contenido del bloque físico origen.
 * Verifica la integridad del bloque clonado usando hash MD5 y reintenta hasta 3 veces en caso de error.
 *
 * @param bloque_fisico_origen: Número del bloque físico a clonar
 * @param g_bitmap: Bitmap de bloques físicos
 * @param g_bitmap_size: Tamaño del bitmap en bytes
 * @param g_block_size: Tamaño de cada bloque en bytes
 * @param g_fs_size: Tamaño total del filesystem en bytes
 * @param punto_montaje: Path del punto de montaje del filesystem
 * @param bitmap_mutex: Mutex para proteger el acceso al bitmap
 *
 * @return: Número del bloque físico clonado (destino), o -1 si no hay bloques disponibles o hay error después de 3 intentos
 */
int clonar_bloque_fisico(int bloque_fisico_origen, t_bitarray* g_bitmap, size_t g_bitmap_size,
                         int g_block_size, int g_fs_size, char* punto_montaje, pthread_mutex_t* bitmap_mutex)
{
    int cantidad_bloques = g_fs_size / g_block_size;
    int bloque_destino = -1;
    char* buffer_origen = NULL;
    char* buffer_destino = NULL;
    char* path_origen = NULL;
    char* path_destino = NULL;
    char* hash_origen = NULL;
    char* hash_destino = NULL;
    int intentos = 0;
    int max_intentos = 3;
    bool clonacion_exitosa = false;

    // Construir path del bloque origen
    path_origen = string_from_format("%s/physical_blocks/block%04d.dat", punto_montaje, bloque_fisico_origen);

    // Leer contenido del bloque origen
    FILE* f_origen = fopen(path_origen, "rb");
    if (f_origen == NULL) {
        log_orange(logger, "[CLONAR_BLOQUE] No se pudo abrir el bloque origen %d en %s", bloque_fisico_origen, path_origen);
        free(path_origen);
        return -1;
    }

    // Reservar buffer para el contenido del origen
    buffer_origen = malloc(g_block_size);
    if (buffer_origen == NULL) {
        log_orange(logger, "[CLONAR_BLOQUE] No se pudo reservar memoria para el buffer de origen");
        fclose(f_origen);
        free(path_origen);
        return -1;
    }

    // Leer contenido del bloque origen
    size_t bytes_leidos = fread(buffer_origen, 1, g_block_size, f_origen);
    fclose(f_origen);

    if (bytes_leidos != g_block_size) {
        log_orange(logger, "[CLONAR_BLOQUE] Se leyeron %zu bytes en lugar de %d del bloque origen", bytes_leidos, g_block_size);
    }

    // Calcular hash MD5 del bloque origen
    hash_origen = crypto_md5(buffer_origen, g_block_size);
    log_orange(logger, "[CLONAR_BLOQUE] Hash del bloque origen %d: %s", bloque_fisico_origen, hash_origen);

    // Intentar clonar hasta 3 veces
    while (intentos < max_intentos && !clonacion_exitosa) {
        intentos++;
        log_orange(logger, "[CLONAR_BLOQUE] Intento %d de %d", intentos, max_intentos);

        // Buscar bloque libre en el bitmap (con mutex)
        pthread_mutex_lock(bitmap_mutex);

        bloque_destino = -1;
        for (int i = 0; i < cantidad_bloques; i++) {
            if (!bloque_ocupado(g_bitmap, i)) {
                bloque_destino = i;
                // Marcar inmediatamente como ocupado para evitar race conditions
                ocupar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
                break;
            }
        }

        pthread_mutex_unlock(bitmap_mutex);

        // Si no hay bloques disponibles
        if (bloque_destino == -1) {
            log_orange(logger, "[CLONAR_BLOQUE] No hay bloques físicos disponibles");
            free(buffer_origen);
            free(path_origen);
            free(hash_origen);
            return -1;
        }

        log_orange(logger, "[CLONAR_BLOQUE] Bloque destino seleccionado: %d", bloque_destino);

        // Construir path del bloque destino
        path_destino = string_from_format("%s/physical_blocks/block%04d.dat", punto_montaje, bloque_destino);

        // Escribir contenido en el bloque destino
        FILE* f_destino = fopen(path_destino, "wb");
        if (f_destino == NULL) {
            log_orange(logger, "[CLONAR_BLOQUE] No se pudo abrir el bloque destino %d en %s", bloque_destino, path_destino);

            // Liberar el bloque que no se pudo usar
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);

            free(path_destino);
            continue; // Reintentar
        }

        size_t bytes_escritos = fwrite(buffer_origen, 1, g_block_size, f_destino);
        fclose(f_destino);

        if (bytes_escritos != g_block_size) {
            log_orange(logger, "[CLONAR_BLOQUE] Error al escribir en el bloque destino %d (escribió %zu bytes)", bloque_destino, bytes_escritos);

            // Liberar el bloque que no se pudo usar
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);

            free(path_destino);
            continue; // Reintentar
        }

        // Leer el bloque destino para verificar
        FILE* f_destino_verificacion = fopen(path_destino, "rb");
        if (f_destino_verificacion == NULL) {
            log_orange(logger, "[CLONAR_BLOQUE] No se pudo abrir el bloque destino para verificación");

            // Liberar el bloque
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);

            free(path_destino);
            continue; // Reintentar
        }

        // Reservar buffer para verificación
        buffer_destino = malloc(g_block_size);
        if (buffer_destino == NULL) {
            log_orange(logger, "[CLONAR_BLOQUE] No se pudo reservar memoria para el buffer de verificación");
            fclose(f_destino_verificacion);

            // Liberar el bloque
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);

            free(path_destino);
            continue; // Reintentar
        }

        size_t bytes_verificacion = fread(buffer_destino, 1, g_block_size, f_destino_verificacion);
        fclose(f_destino_verificacion);

        if (bytes_verificacion != g_block_size) {
            log_orange(logger, "[CLONAR_BLOQUE] Error al leer el bloque destino para verificación");
            free(buffer_destino);

            // Liberar el bloque
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);

            free(path_destino);
            continue; // Reintentar
        }

        // Calcular hash MD5 del bloque destino
        hash_destino = crypto_md5(buffer_destino, g_block_size);
        log_orange(logger, "[CLONAR_BLOQUE] Hash del bloque destino %d: %s", bloque_destino, hash_destino);

        // Comparar hashes
        if (string_equals_ignore_case(hash_origen, hash_destino)) {
            clonacion_exitosa = true;
            log_orange(logger, "[CLONAR_BLOQUE] Bloque físico %d clonado exitosamente al bloque %d (intento %d)",
                       bloque_fisico_origen, bloque_destino, intentos);
        } else {
            log_orange(logger, "[CLONAR_BLOQUE] Los hashes no coinciden. Intento %d fallido", intentos);
            log_orange(logger, "[CLONAR_BLOQUE] Hash origen:  %s", hash_origen);
            log_orange(logger, "[CLONAR_BLOQUE] Hash destino: %s", hash_destino);

            // Liberar el bloque porque la clonación falló
            pthread_mutex_lock(bitmap_mutex);
            liberar_bloque(g_bitmap, bloque_destino, g_bitmap_size);
            pthread_mutex_unlock(bitmap_mutex);
        }

        // Limpiar recursos del intento
        free(buffer_destino);
        buffer_destino = NULL;
        free(hash_destino);
        hash_destino = NULL;
        free(path_destino);
        path_destino = NULL;
    }

    // Liberar recursos finales
    free(buffer_origen);
    free(path_origen);
    free(hash_origen);

    if (clonacion_exitosa) {
        return bloque_destino;
    } else {
        log_orange(logger, "[CLONAR_BLOQUE] Error: No se pudo clonar el bloque después de %d intentos", max_intentos);
        return -1;
    }
}

#endif