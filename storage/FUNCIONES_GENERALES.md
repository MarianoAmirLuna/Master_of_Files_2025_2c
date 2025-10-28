# Documentación de Funciones Generales - Storage

Este documento contiene la descripción completa de todas las funciones generales utilizadas en el módulo Storage del sistema de archivos.

---

## Índice

1. [Funciones de Directorio](#funciones-de-directorio)
2. [Funciones de Verificación de Existencia](#funciones-de-verificación-de-existencia)
3. [Funciones de Archivo](#funciones-de-archivo)
4. [Funciones de Bloques Físicos](#funciones-de-bloques-físicos)
5. [Funciones de Bitmap](#funciones-de-bitmap)
6. [Funciones de Metadata](#funciones-de-metadata)
7. [Funciones de Hard Links](#funciones-de-hard-links)

---

## Funciones de Directorio

### `crear_directorio`

```c
void crear_directorio(char* nombre, char* path)
```

**Descripción:**
Crea un directorio en el sistema de archivos.

**Parámetros:**
- `nombre` (char*): Nombre del directorio a crear
- `path` (char*): Ruta donde se creará el directorio

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Crea el directorio con permisos 0777
- Registra un log cuando el directorio se crea exitosamente
- Libera automáticamente la memoria del path concatenado

**Ejemplo:**
```c
crear_directorio("mi_carpeta", "/home/utnso/storage");
// Crea: /home/utnso/storage/mi_carpeta
```

---

### `cant_elementos_directorio`

```c
int cant_elementos_directorio(char* path)
```

**Descripción:**
Cuenta la cantidad de elementos (archivos y subdirectorios) dentro de un directorio, excluyendo `.` y `..`.

**Parámetros:**
- `path` (char*): Ruta del directorio a analizar

**Retorna:**
- `int`: Cantidad de elementos en el directorio
- `-1`: Si hubo un error al abrir el directorio

**Ejemplo:**
```c
int cantidad = cant_elementos_directorio("/home/utnso/storage/files");
printf("El directorio tiene %d elementos\n", cantidad);
```

---

## Funciones de Verificación de Existencia

### `control_existencia`

```c
int control_existencia(char* path)
```

**Descripción:**
Verifica si existe un archivo o directorio en la ruta especificada usando la syscall `stat()`.

**Parámetros:**
- `path` (char*): Ruta del archivo o directorio a verificar

**Retorna:**
- `true` (1): Si el path existe
- `false` (0): Si el path no existe o es NULL

**Ejemplo:**
```c
if (control_existencia("/home/utnso/storage/files/archivo1")) {
    printf("El archivo existe\n");
}
```

---

### `control_existencia_file`

```c
int control_existencia_file(char* path)
```

**Descripción:**
Verifica si existe un archivo específico intentando abrirlo con `fopen()`. Genera logs informativos durante la verificación.

**Parámetros:**
- `path` (char*): Ruta completa del archivo a verificar

**Retorna:**
- `1`: Si el archivo existe y se puede abrir
- `0`: Si el archivo no existe o es NULL

**Comportamiento:**
- Registra el path en un log de warning
- Registra si el archivo existe o no

**Ejemplo:**
```c
if (control_existencia_file("/home/utnso/storage/metadata.config")) {
    printf("El archivo metadata.config existe\n");
}
```

---

### `tag_comiteado`

```c
bool tag_comiteado(char* file, char* tag)
```

**Descripción:**
Verifica si un tag de un archivo ha sido comiteado (estado COMMITED).

**Parámetros:**
- `file` (char*): Nombre del archivo
- `tag` (char*): Nombre del tag a verificar

**Retorna:**
- `true` (1): Siempre retorna true (implementación incompleta)

**Nota:**
⚠️ Esta función tiene una implementación temporal y siempre retorna `true`.

---

## Funciones de Archivo

### `crear_archivo`

```c
int crear_archivo(char* path, char* nombre, char* extension)
```

**Descripción:**
Crea un archivo vacío con la extensión especificada.

**Parámetros:**
- `path` (char*): Ruta donde se creará el archivo
- `nombre` (char*): Nombre del archivo sin extensión
- `extension` (char*): Extensión del archivo (sin el punto)

**Retorna:**
`int` - Código de resultado (implementación actual no retorna valor explícito)

**Comportamiento:**
- Solo crea el archivo si no existe previamente
- Si el archivo ya existe, registra un warning

**Ejemplo:**
```c
crear_archivo("/home/utnso/storage", "test", "txt");
// Crea: /home/utnso/storage/test.txt
```

---

### `eliminar_archivo`

```c
void eliminar_archivo(char* path, char* nombre)
```

**Descripción:**
Elimina un archivo del sistema de archivos.

**Parámetros:**
- `path` (char*): Ruta donde se encuentra el archivo
- `nombre` (char*): Nombre completo del archivo (con extensión)

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Verifica que el archivo exista antes de eliminarlo
- Registra un error si el archivo no existe

**Ejemplo:**
```c
eliminar_archivo("/home/utnso/storage", "test.txt");
```

---

### `llenar_archivo_con_ceros`

```c
void llenar_archivo_con_ceros(char* path_archivo, int g_block_size)
```

**Descripción:**
Llena un archivo con el carácter '0' (cero ASCII) hasta el tamaño especificado.

**Parámetros:**
- `path_archivo` (char*): Ruta completa del archivo a llenar
- `g_block_size` (int): Tamaño en bytes a llenar (generalmente el tamaño del bloque)

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Crea un buffer temporal del tamaño especificado
- Llena el buffer con el carácter '0'
- Escribe el buffer en el archivo
- Libera automáticamente la memoria del buffer

**Ejemplo:**
```c
llenar_archivo_con_ceros("/home/utnso/storage/physical_blocks/block0000.dat", 128);
// Llena el bloque con 128 caracteres '0'
```

---

## Funciones de Bloques Físicos

### `crear_bloques_fisicos`

```c
void crear_bloques_fisicos(int cantidad_bloques_faltantes, char* path, int bloques_actuales, int g_block_size)
```

**Descripción:**
Crea múltiples bloques físicos en el sistema de archivos.

**Parámetros:**
- `cantidad_bloques_faltantes` (int): Cantidad de bloques a crear
- `path` (char*): Ruta donde se crearán los bloques
- `bloques_actuales` (int): Número del primer bloque a crear (offset)
- `g_block_size` (int): Tamaño de cada bloque en bytes

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Crea archivos con nombre `blockXXXX.dat` donde XXXX es un número de 4 dígitos
- Llena cada bloque con caracteres '0'
- Los bloques se numeran secuencialmente desde `bloques_actuales`

**Ejemplo:**
```c
crear_bloques_fisicos(5, "/home/utnso/storage/physical_blocks", 0, 128);
// Crea: block0000.dat, block0001.dat, block0002.dat, block0003.dat, block0004.dat
```

---

### `eliminar_bloques_fisicos`

```c
void eliminar_bloques_fisicos(int cantidad_bloques_de_mas, char* path, int bloques_actuales)
```

**Descripción:**
Elimina bloques físicos sobrantes del sistema de archivos.

**Parámetros:**
- `cantidad_bloques_de_mas` (int): Cantidad de bloques a eliminar
- `path` (char*): Ruta donde se encuentran los bloques
- `bloques_actuales` (int): Número del último bloque actual

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Elimina bloques en orden inverso (desde el último hacia atrás)
- Los bloques se nombran con formato `blockXXXX.dat`

**Ejemplo:**
```c
eliminar_bloques_fisicos(2, "/home/utnso/storage/physical_blocks", 10);
// Elimina block0010.dat y block0009.dat
```

---

### `escribir_bloque_fisico`

```c
void escribir_bloque_fisico(int bloque_fisico, char* contenido)
```

**Descripción:**
Escribe contenido en un bloque físico específico.

**Parámetros:**
- `bloque_fisico` (int): Número del bloque físico a escribir
- `contenido` (char*): Contenido a escribir en el bloque

**Retorna:**
`void` - No retorna ningún valor

**Nota:**
⚠️ Esta función tiene un bug en la línea 267: `if( f = NULL )` debería ser `if( f == NULL )`.

---

### `clonar_bloque_fisico`

```c
int clonar_bloque_fisico(int bloque_fisico_origen, t_bitarray* g_bitmap, size_t g_bitmap_size,
                         int g_block_size, int g_fs_size, char* punto_montaje, pthread_mutex_t* bitmap_mutex)
```

**Descripción:**
Clona un bloque físico a un nuevo bloque libre, verificando la integridad mediante hash MD5. Reintenta hasta 3 veces en caso de error.

**Parámetros:**
- `bloque_fisico_origen` (int): Número del bloque físico a clonar
- `g_bitmap` (t_bitarray*): Bitmap de bloques físicos
- `g_bitmap_size` (size_t): Tamaño del bitmap en bytes
- `g_block_size` (int): Tamaño de cada bloque en bytes
- `g_fs_size` (int): Tamaño total del filesystem en bytes
- `punto_montaje` (char*): Path del punto de montaje del filesystem
- `bitmap_mutex` (pthread_mutex_t*): Mutex para proteger el acceso al bitmap

**Retorna:**
- `int >= 0`: Número del bloque físico clonado (destino)
- `-1`: Si no hay bloques disponibles o hay error después de 3 intentos

**Comportamiento:**
1. Lee el contenido del bloque origen
2. Calcula el hash MD5 del bloque origen
3. Busca un bloque libre en el bitmap (con protección mutex)
4. Copia el contenido al bloque destino
5. Calcula el hash MD5 del bloque destino
6. Compara ambos hashes para verificar integridad
7. Reintenta hasta 3 veces si los hashes no coinciden
8. Marca el bloque destino como ocupado en el bitmap

**Ejemplo:**
```c
int nuevo_bloque = clonar_bloque_fisico(5, g_bitmap, g_bitmap_size,
                                         128, 4096, "/home/utnso/storage", &bitmap_lock);
if (nuevo_bloque != -1) {
    printf("Bloque 5 clonado al bloque %d\n", nuevo_bloque);
}
```

**Logs generados:**
- `[CLONAR_BLOQUE] Hash del bloque origen X: <hash>`
- `[CLONAR_BLOQUE] Intento Y de 3`
- `[CLONAR_BLOQUE] Bloque físico X clonado exitosamente al bloque Y (intento Z)`

---

## Funciones de Bitmap

### `ocupar_bloque`

```c
void ocupar_bloque(t_bitarray* g_bitmap, off_t nro_bloque, size_t g_bitmap_size)
```

**Descripción:**
Marca un bloque físico como ocupado en el bitmap y sincroniza los cambios a disco.

**Parámetros:**
- `g_bitmap` (t_bitarray*): Bitmap de bloques físicos
- `nro_bloque` (off_t): Número del bloque a marcar como ocupado
- `g_bitmap_size` (size_t): Tamaño del bitmap en bytes

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Setea el bit correspondiente en el bitmap
- Sincroniza el bitmap a disco usando `msync()`
- Registra un log de debug

**Ejemplo:**
```c
ocupar_bloque(g_bitmap, 5, g_bitmap_size);
// Marca el bloque 5 como ocupado
```

---

### `liberar_bloque`

```c
void liberar_bloque(t_bitarray* g_bitmap, off_t nro_bloque, size_t g_bitmap_size)
```

**Descripción:**
Marca un bloque físico como libre en el bitmap y sincroniza los cambios a disco.

**Parámetros:**
- `g_bitmap` (t_bitarray*): Bitmap de bloques físicos
- `nro_bloque` (off_t): Número del bloque a marcar como libre
- `g_bitmap_size` (size_t): Tamaño del bitmap en bytes

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Limpia el bit correspondiente en el bitmap
- Sincroniza el bitmap a disco usando `msync()`
- Registra un log de debug

**Ejemplo:**
```c
liberar_bloque(g_bitmap, 5, g_bitmap_size);
// Marca el bloque 5 como libre
```

---

### `bloque_ocupado`

```c
bool bloque_ocupado(t_bitarray* g_bitmap, int nro_bloque)
```

**Descripción:**
Verifica si un bloque físico está ocupado en el bitmap.

**Parámetros:**
- `g_bitmap` (t_bitarray*): Bitmap de bloques físicos
- `nro_bloque` (int): Número del bloque a verificar

**Retorna:**
- `true`: Si el bloque está ocupado
- `false`: Si el bloque está libre

**Ejemplo:**
```c
if (bloque_ocupado(g_bitmap, 5)) {
    printf("El bloque 5 está ocupado\n");
}
```

---

### `destruir_bitmap`

```c
void destruir_bitmap(t_bitarray* g_bitmap, int g_bitmap_fd)
```

**Descripción:**
Destruye el bitmap y cierra el file descriptor asociado.

**Parámetros:**
- `g_bitmap` (t_bitarray*): Bitmap a destruir
- `g_bitmap_fd` (int): File descriptor del archivo de bitmap

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Libera la memoria del bitarray
- Cierra el file descriptor del bitmap

**Ejemplo:**
```c
destruir_bitmap(g_bitmap, g_bitmap_fd);
```

---

## Funciones de Metadata

### `crear_metadata_config`

```c
t_config* crear_metadata_config(char* path, int tamanio, t_list* bloques, state_metadata estado)
```

**Descripción:**
Crea o actualiza el archivo metadata.config de un File:Tag con el tamaño, lista de bloques físicos y estado.

**Parámetros:**
- `path` (char*): Ruta completa del archivo metadata.config
- `tamanio` (int): Tamaño del File:Tag en bytes
- `bloques` (t_list*): Lista de punteros a int con los números de bloques físicos
- `estado` (state_metadata): Estado del File:Tag (WORK_IN_PROGRESS o COMMITED)

**Retorna:**
- `t_config*`: Puntero a la configuración creada/actualizada

**Comportamiento:**
- Si el archivo no existe, lo crea
- Si existe, lo actualiza
- Guarda las siguientes propiedades:
  - `TAMAÑO`: Tamaño del archivo en bytes
  - `BLOCKS`: Lista de bloques en formato `[1,2,3,4]`
  - `ESTADO`: Estado (WORK_IN_PROGRESS o COMMITED)
- Sincroniza los cambios a disco automáticamente

**Ejemplo:**
```c
t_list* bloques = list_create();
int* b1 = malloc(sizeof(int)); *b1 = 0;
int* b2 = malloc(sizeof(int)); *b2 = 1;
list_add(bloques, b1);
list_add(bloques, b2);

t_config* metadata = crear_metadata_config(
    "/home/utnso/storage/files/archivo1/tag1/metadata.config",
    256,
    bloques,
    WORK_IN_PROGRESS
);

// Genera:
// TAMAÑO=256
// BLOCKS=[0,1]
// ESTADO=WORK_IN_PROGRESS
```

---

### `crear_config_path`

```c
void crear_config_path(char* p_path, char* name)
```

**Descripción:**
Crea el archivo blocks_hash_index.config en la ruta especificada.

**Parámetros:**
- `p_path` (char*): Ruta donde se creará el archivo
- `name` (char*): Nombre del archivo de configuración

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Concatena el path y el nombre
- Llama a `create_blocks_hash_index()` para crear el archivo

**Ejemplo:**
```c
crear_config_path("/home/utnso/storage", "blocks_hash_index.config");
```

---

## Funciones de Hard Links

### `crear_hard_link`

```c
void crear_hard_link(char* path_bloque_fisico, char* path_bloque_logico)
```

**Descripción:**
Crea un hard link desde un bloque lógico a un bloque físico usando la syscall `link()`.

**Parámetros:**
- `path_bloque_fisico` (char*): Ruta del bloque físico existente
- `path_bloque_logico` (char*): Ruta del bloque lógico a crear (hard link)

**Retorna:**
`void` - No retorna ningún valor

**Comportamiento:**
- Crea un hard link del bloque lógico apuntando al bloque físico
- Registra un log de debug si se crea exitosamente
- Registra un log de error si falla la operación

**Ejemplo:**
```c
crear_hard_link(
    "/home/utnso/storage/physical_blocks/block0005.dat",
    "/home/utnso/storage/files/archivo1/tag1/logical_blocks/000000.dat"
);
// Crea el hard link: 000000.dat -> block0005.dat
```

---

## Notas Importantes

### Thread Safety

Las siguientes funciones requieren protección con mutex cuando se usan en un entorno multihilo:

- `ocupar_bloque()` - Requiere `bitmap_lock`
- `liberar_bloque()` - Requiere `bitmap_lock`
- `bloque_ocupado()` - Requiere `bitmap_lock` para lectura consistente
- `clonar_bloque_fisico()` - Ya incluye protección interna del bitmap

### Gestión de Memoria

- Las funciones que reciben punteros `char*` NO liberan la memoria automáticamente (excepto `crear_directorio` que libera el path concatenado internamente)
- `crear_metadata_config()` libera internamente la memoria de `bloques_str`
- El usuario debe liberar la memoria de las listas y sus elementos después de usar `crear_metadata_config()`

### Buenas Prácticas

1. Siempre verificar existencia antes de operar sobre archivos
2. Usar `clonar_bloque_fisico()` en lugar de copiar manualmente bloques
3. Proteger accesos al bitmap con `bitmap_lock`
4. Liberar memoria de listas y punteros después de usarlos
5. Verificar valores de retorno de funciones que pueden fallar

---

## Dependencias

Las funciones utilizan las siguientes bibliotecas:

- `commons/config.h` - Para manejo de archivos de configuración
- `commons/string.h` - Para manipulación de strings
- `commons/bitarray.h` - Para manejo del bitmap
- `commons/collections/list.h` - Para listas dinámicas
- `sys/stat.h` - Para verificación de archivos
- `sys/mman.h` - Para memoria mapeada (mmap/msync)
- `pthread.h` - Para mutex y sincronización

---

**Última actualización:** 2025-10-28
**Versión:** 1.0
