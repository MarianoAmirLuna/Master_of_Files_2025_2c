#ifndef STORAGE_BASE_H
#include "base.h"
#endif 

/*
Inicialización
Al momento de inicializar el FS lo primero que se debe verificar es el valor FRESH_START del archivo de configuración.
Este valor nos indicará si se debe formatear el volumen (iniciando un FS nuevo) o si se quiere mantener el contenido preexistente.

Al inicializar desde cero el FS (FRESH_START=TRUE), el único archivo nativo que necesitamos tener obligatoriamente es el archivo superblock.config.
Ese archivo nos indicará los datos necesarios para poder formatear nuestro FS,
eliminando previamente los demás archivos que componen nuestro FS en caso de existir.

También se deberá crear un primer File llamado "initial_file" confirmado con un único Tag "BASE",
cuyo contenido será un (1) bloque lógico con el bloque físico nro 0 (cero) asignado, completando el bloque con el caracter 0,
por ejemplo: "00000…". Dicho File/Tag no se podrá borrar.
*/



void limpiar_fs() 
{
    eliminar_contenido(cs.punto_montaje);
}



void eliminar_contenido(const char* path) {
    DIR* dir = opendir(path);
    struct dirent* entry;

    if (dir == NULL) {
        perror("No se pudo abrir el directorio");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Ignorar "." y ".."
        if (string_equals_ignore_case(entry->d_name, ".") || string_equals_ignore_case(entry->d_name, ".."))
            continue;

        // Ignorar el archivo superblock.config
        if (string_equals_ignore_case(entry->d_name, "superblock.config"))
            continue;

        // Construir path completo
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);

        struct stat statbuf;
        if (stat(full_path, &statbuf) == -1) {
            perror("No se pudo obtener información del archivo");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode)) {
            // Es un directorio: llamar recursivamente y luego eliminarlo
            eliminar_contenido(full_path);
            if (rmdir(full_path) != 0)
                perror("Error al eliminar directorio");
        } else {
            // Es un archivo
            if (remove(full_path) != 0)
                perror("Error al eliminar archivo");
        }
    }

    closedir(dir);
}



void valido_inicial_file(char* path)
{
    // Controlo que exista cada directorio, y en caso que no exista se creada
    // Hay que integrar aca dentro la creacion del meta.config y el 000000.dat, y en el caso de que exista el directorio veriticar la existencia de dichos archivos.
    char* p_path = string_from_format("%s/%s", path, "files");

    if (!control_existencia(p_path))
    {
        crear_directorio("files", path);
        free(p_path);
    }
    p_path = string_from_format("%s/%s", path, "files/initial_file");
    if (!control_existencia(p_path))
    {
        crear_directorio("files/initial_file", path);
        free(p_path);
    }
    p_path = string_from_format("%s/%s", path, "files/initial_file/BASE");
    if (!control_existencia(p_path))
    {
        free(p_path);
        p_path = string_from_format("%s/%s", path, "files/initial_file");
        crear_directorio("BASE",p_path);
        free(p_path);
    }
    p_path = string_from_format("%s/%s", path, "files/initial_file/BASE/metadata.config");
    if (!control_existencia(p_path))
    {
        //crear_config_block_hash_index(string_from_format("%s/%s", path, "files/initial_file/BASE/"), "metadata.config"); todo: borrar despues
        // Armo lista de bloques inicial
        t_list* bloques = list_create();
        int* b0 = malloc(sizeof(int));
        *b0 = 0;
        list_add(bloques, b0);
        free(p_path);
        char* meta_path = string_from_format("%s/%s", path, "files/initial_file/BASE/metadata.config");
        crear_metadata_config(meta_path, g_block_size, bloques, COMMITED);

        list_destroy_and_destroy_elements(bloques, free);
        free(meta_path);
    }    
    p_path = string_from_format("%s/%s", path, "files/initial_file/BASE/logical_blocks");
    if (!control_existencia(p_path))
    {
        free(p_path);
        p_path = string_from_format("%s/%s", path, "files/initial_file/BASE/");
        crear_directorio("logical_blocks",p_path);
        //Se elimina el crear_archivo porque la funcion link genera el archivo para crear el hardlink
        char* p2_path = string_from_format("%s/%s", path, "files/initial_file/BASE/logical_blocks/000000.dat");
        p_path = string_from_format("%s/%s", path, "physical_blocks/block0000.dat");
        crear_hard_link(p_path, p2_path);
        free(p_path);
        free(p2_path);
    }
}

void inicializar_bitmap(const char* punto_montaje) {
    size_t num_blocks = g_fs_size / g_block_size;
    g_bitmap_size = (size_t) ceil((double) num_blocks / 8);

    char* path = string_from_format("%s/bitmap.bin", punto_montaje);

    // creo bitmap
    g_bitmap_fd = open(path, O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (g_bitmap_fd == -1) {
        perror("open bitmap.bin");
        free(path);
        return;
    }
    ftruncate(g_bitmap_fd, g_bitmap_size);

    // mapeo en memoria
    void* bitmap_data = mmap(NULL, g_bitmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, g_bitmap_fd, 0);
    if (bitmap_data == MAP_FAILED) {
        perror("mmap");
        close(g_bitmap_fd);
        free(path);
        return;
    }

    // bitarray
    g_bitmap = bitarray_create_with_mode((char*) bitmap_data, g_bitmap_size, LSB_FIRST);

    // si es fresh_start limpio
    if (cs.fresh_start) {
        memset(g_bitmap->bitarray, 0, g_bitmap_size);
        msync(g_bitmap->bitarray, g_bitmap_size, MS_SYNC);
        log_info(logger, "Bitmap inicializado vacío con %zu bloques", num_blocks);
    } else {
        log_info(logger, "Bitmap cargado desde archivo existente");
    }

    free(path);
}


void valido_bitmap(const char* punto_montaje) {
    inicializar_bitmap(punto_montaje);
}


// valido que exista el archivo blokcs_hash_index.config, si no existe lo creo en blanco
void valido_hash(const char* p_path)
{
    char* path = string_from_format("%s/%s", p_path, "blocks_hash_index.config");
    if (!control_existencia_file(path))
    {
        log_orange(logger, "[valido_hash] No se encontro el archivo blocks_hash_index en el path %s, creando uno nuevo", path); // to_do: sacar despues
        create_blocks_hash_index(path);
    }
    else
    {
        log_orange(logger, "[valido_hash] Se encontro el archivo blocks_hash_index en el path %s", path); // to_do: sacar despues
    }
    free(path);
}

void valido_bloques_fisicos(char* path)
{
    //Controlo que exista el directorio de bloques fisicos
    char* p_path = string_from_format("%s/%s", path, "physical_blocks");
    if (control_existencia(p_path))
    {
        free(p_path);
        log_light_green(logger, "[valido_bloques_fisicos] Directorio encontrado, verificando que el block count sea el correcto"); //to_do: borrar este
        int cantidad_bloques = g_fs_size / g_block_size;
        log_light_blue(logger, "[valido_bloques_fisicos] Cantidad de bloques que deberia haber: %d", cantidad_bloques); //to_do: borrar este

        //Veriifico la cantidad de bloques fisicos que hay
        p_path = string_from_format("%s/%s", path, "physical_blocks");
        if (cant_elementos_directorio(p_path) == cantidad_bloques)
        {
            free(p_path);
            //Cantidad de bloques correcta, por lo tanto no no hay que crear nuevos bloques
            log_light_green(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es correcta"); //to_do: borrar este
        }
        else
        {
            //Cantidad de bloques incorrecta, por lo tanto hay que crear los bloques faltantes o eliminar los bloques de mas

            //Cantidad de bloques inferior
            p_path = string_from_format("%s/%s", path, "physical_blocks");
            do 
            {
                int bloques_restantes = cantidad_bloques - cant_elementos_directorio(p_path);
                log_orange(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es INFERIOR, faltan %d bloques", bloques_restantes); //to_do: borrar este
                crear_bloques_fisicos (bloques_restantes, p_path, cant_elementos_directorio(p_path), g_block_size);
            }
            while(cant_elementos_directorio(p_path) < cantidad_bloques);

            //Cantidad de bloques superior
            do 
            {
                int bloques_restantes = cantidad_bloques - cant_elementos_directorio(p_path);
                log_orange(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es SUPERIOR, sobran %d bloques", bloques_restantes); //to_do: borrar este
                eliminar_bloques_fisicos(bloques_restantes, p_path, cant_elementos_directorio(p_path));
            }
            while(cant_elementos_directorio(p_path) > cantidad_bloques);
            free(p_path);
        }
        
    }
    else
    {
        log_light_blue(logger, "[valido_bloques_fisicos] No se encontro el directorio physical_blocks, creando el directorio y los bloques fisicos"); //to_do: borrar este
        int cantidad_bloques = g_fs_size / g_block_size;
        crear_directorio("physical_blocks", path);
        p_path = string_from_format("%s/%s", path, "physical_blocks");
        crear_bloques_fisicos(cantidad_bloques, p_path, 0, g_block_size);
        free(p_path);
    }

}

void inicializar_file_system()
{
    //TODO: Por defecto debe crear "superblock.config", "bitmap.bin", "block_hash_index.config"
    //y crear directorio desde punto de montaje el "physical_blocks" y "files" y si no me equivoco también el "initial_file" dentro del "files"
    
    
    //Y si no existe el superblock que haces???... Gato mulo
    char* path_superblock_config = string_from_format("%s/%s", cs.punto_montaje, "superblock.config");
    t_config* c_superblock = config_create(path_superblock_config);

    g_block_size = config_get_int_value(c_superblock, "BLOCK_SIZE");
    g_fs_size = config_get_int_value(c_superblock, "FS_SIZE");

    if (cs.fresh_start) // si es 1 (true)
    {
        log_pink(logger, "limpio el storage"); //to_do: borrar
        limpiar_fs();
    }
    else // si es 0 (false)
    {
        log_pink(logger, "no limpio el storage"); //to_do: borrar
    }
    valido_bitmap(cs.punto_montaje);
    valido_hash(cs.punto_montaje);
    valido_bloques_fisicos(cs.punto_montaje);
    valido_inicial_file(cs.punto_montaje);
}


