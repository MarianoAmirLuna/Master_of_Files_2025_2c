#include "base.h"

void crear_directorio(char* nombre, char* path);
bool control_existencia(const char* path);
bool control_existencia_file(const char* path);
int cant_elementos_directorio (const char *path);
void crear_archivo (const char* path, const char* nombre, const char* extension);
void eliminar_archivo (const char* path, const char* nombre);
void crear_hard_link(char* path_bloque_fisico, char* path_bloque_logico);
void crear_config_path(char* p_path, char* name);
t_config* crear_metadata_config(char* path, int tamanio, t_list* bloques, state_metadata estado);
void llenar_archivo_con_ceros(char* path_archivo);
void eliminar_bloques_fisicos (int cantidad_bloques_de_mas, char* path, int bloques_actuales);
void crear_bloques_fisicos (int cantidad_bloques_faltantes, char* path, int bloques_actuales);
