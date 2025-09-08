#include "base.h"

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


void inicializar_file_system()
{
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


void valido_inicial_file(char* path)
{
    // Controlo que exista cada directorio, y en caso que no exista se creada
    // Hay que integrar aca dentro la creacion del meta.config y el 000000.dat, y en el caso de que exista el directorio veriticar la existencia de dichos archivos.
    if (!control_existencia(string_from_format("%s/%s", path, "files")))
    {
        crear_directorio("files", path);
    }
    if (!control_existencia(string_from_format("%s/%s", path, "files/initial_file")))
    {
        crear_directorio("files/initial_file", path);
    }
    if (!control_existencia(string_from_format("%s/%s", path, "files/initial_file/BASE")))
    {
        crear_directorio("BASE",string_from_format("%s/%s", path, "files/initial_file/"));
    }
    if (!control_existencia(string_from_format("%s/%s", path, "files/initial_file/BASE/metadata.config")))
    {
        //crear_config_path(string_from_format("%s/%s", path, "files/initial_file/BASE/"), "metadata.config");
            // Armo lista de bloques inicial
        t_list* bloques = list_create();
        int* b0 = malloc(sizeof(int));
        *b0 = 0;
        list_add(bloques, b0);

        char* meta_path = string_from_format("%s/%s", path, "files/initial_file/BASE/metadata.config");
        crear_metadata_config(meta_path, g_block_size, bloques, COMMITED);

        list_destroy_and_destroy_elements(bloques, free);
        free(meta_path);
    }    
    if (!control_existencia(string_from_format("%s/%s", path, "files/initial_file/BASE/logical_blocks")))
    {
        crear_directorio("logical_blocks",string_from_format("%s/%s", path, "files/initial_file/BASE/"));
        //Se elimina el crear_archivo porque la funcion link genera el archivo para crear el hardlink
        crear_hard_link(string_from_format("%s/%s", path, "physical_blocks/block0000.dat"), string_from_format("%s/%s", path, "files/initial_file/BASE/logical_blocks/000000.dat"));
    }
}

void valido_bitmap(const char* p_path)
{
    char* path = string_from_format("%s/%s", p_path, "bitmap.bin");
    FILE* bitmap = fopen(path, "rb");
    // valido si existe el archivo 
    if(bitmap == NULL)
    {
        log_debug(logger, "No se encontro el archivo bitmap en el path %s", path); // to_do: sacar despues
        bitmap = fopen(path, "wb");
        if(bitmap == NULL){
            log_error(logger, "Error raro al crear el bitmap"); // to_do: sacar despues.
        }
        inicializar_bitmap();
    }
}


void inicializar_bitmap() {
    size_t num_blocks = g_fs_size / g_block_size;

    size_t bitmap_size = (size_t) ceil((double) num_blocks / 8);

    char* path = string_from_format("%s/%s", cs.punto_montaje, "bitmap.bin");
    FILE* bitmap = fopen(path, "wb");
    if (bitmap == NULL) {
        log_error(logger, "Error al crear el archivo bitmap"); //to_do: sacar despues
        return;
    }

    // crear un bitmap vacío (todos los bits en 0)
    unsigned char* bitmap_data = (unsigned char*) calloc(bitmap_size, sizeof(unsigned char));

    if (bitmap_data == NULL) {
        log_error(logger, "Error al reservar memoria para el bitmap");
        fclose(bitmap);
        return;
    }

    // Escribir el bitmap vacío en el archivo
    size_t bytes_written = fwrite(bitmap_data, sizeof(unsigned char), bitmap_size, bitmap);
    if (bytes_written != bitmap_size) {
        log_error(logger, "Error al escribir el bitmap en el archivo");//to_do: sacar despues
    } else {
        log_debug(logger, "Bitmap creado correctamente", "bitmap.bin");//to_do: sacar despues
    }

    free(bitmap_data);

    fclose(bitmap);
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


void valido_bloques_fisicos(const char* path)
{
    //Controlo que exista el directorio de bloques fisicos
    if (control_existencia(string_from_format("%s/%s", path, "physical_blocks")))
    {
        log_light_green(logger, "[valido_bloques_fisicos] Directorio encontrado, verificando que el block count sea el correcto"); //to_do: borrar este
        int cantidad_bloques = g_fs_size / g_block_size;
        log_light_blue(logger, "[valido_bloques_fisicos] Cantidad de bloques que deberia haber: %d", cantidad_bloques); //to_do: borrar este

        //Veriifico la cantidad de bloques fisicos que hay
        if (cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks")) == cantidad_bloques)
        {
            //Cantidad de bloques correcta, por lo tanto no no hay que crear nuevos bloques
            log_light_green(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es correcta"); //to_do: borrar este
        }
        else
        {
            //Cantidad de bloques incorrecta, por lo tanto hay que crear los bloques faltantes o eliminar los bloques de mas

            //Cantidad de bloques inferior
            do 
            {
                int bloques_restantes = cantidad_bloques - cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks"));
                log_orange(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es INFERIOR, faltan %d bloques", bloques_restantes); //to_do: borrar este
                crear_bloques_fisicos (bloques_restantes, string_from_format("%s/%s", path, "physical_blocks"), cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks")));
            }
            while(cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks")) < cantidad_bloques);

            //Cantidad de bloques superior
            do 
            {
                int bloques_restantes = cantidad_bloques - cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks"));
                log_orange(logger, "[valido_bloques_fisicos] La cantidad de bloques fisicos es SUPERIOR, sobran %d bloques", bloques_restantes); //to_do: borrar este
                eliminar_bloques_fisicos(bloques_restantes, string_from_format("%s/%s", path, "physical_blocks"), cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks")));
            }
            while(cant_elementos_directorio(string_from_format("%s/%s", path, "physical_blocks")) > cantidad_bloques);

        }
        
    }
    else
    {
        log_light_blue(logger, "[valido_bloques_fisicos] No se encontro el directorio physical_blocks, creando el directorio y los bloques fisicos"); //to_do: borrar este
        int cantidad_bloques = g_fs_size / g_block_size;
        crear_directorio("physical_blocks", path);
        crear_bloques_fisicos (cantidad_bloques, string_from_format("%s/%s", path, "physical_blocks"), 0);
    }

}



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