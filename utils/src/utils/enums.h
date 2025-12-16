#ifndef UTILS_ENUMS_H
#define UTILS_ENUMS_H


typedef enum{
    STATE_READY,
    STATE_EXEC,
    STATE_EXIT
}state_process;

typedef enum{
    PACKET = STATE_EXIT+1,
    INSTRUCTIONS,
    STATUS,
    REQUEST
}op_code;

typedef enum{
    MODULE_QUERY_CONTROL=REQUEST+1,
    MODULE_MASTER,
    MODULE_WORKER,
    MODULE_STORAGE
}op_code_module;

typedef enum{
    DT_STRING=MODULE_STORAGE+1,
    DT_INT,
    DT_UNSIGNEDINT,
    DT_SHORT,
    DT_BYTE
}datatype;

typedef enum{
    SHORT_SCHEDULER=DT_BYTE+1,
    MEDIUM_SCHEDULER,
    LONG_SCHEDULER
}scheduler_mode;

typedef enum{
    /// @brief First in First Out
    FIFO=LONG_SCHEDULER+1,
    /// @brief Prioridades con desalojo y aging
    PRIORITIES,
}scheduler_algorithm;

typedef enum{
    /// @brief OK
    SUCCESS = PRIORITIES+1,
    /// @brief EOL o Fuera de línea cuando no se puede recibir más líneas de un archivo. Típicamente usado en Pseudocódigos    
    ENDOFLINE,
    /// @brief Informa desde IO al Kernel que terminó la solicitud IO
    END_OF_IO,
    /// @brief Interrupción
    INTERRUPT,
    SEGMENTATION_FAULT,
    PROCESS_CANT_INITIALIZED,
    ACTUAL_STATUS,
    /// @brief No existe
    NOTEXISTS,
    /// @brief Replanificación
    RESCHEDULE,
    QUERY_ID,
    /// @brief El proceso terminó de ejecutarse
    PROCESS_FINISHED,
    /// @brief Error general
    ERROR,
}response;

typedef enum{
	REQUEST_EXECUTE_QUERY= ERROR+1,
    REQUEST_END_QUERY,
    REQUEST_CONTEXT_EXECUTION,
    REQUEST_LIST_INSTRUCTIONS,
    REQUEST_CHECK_SPACE_MEMORY,
	REQUEST_INSTRUCTIONS_MEMORY,
    /// @brief Cuando el Master debe desalojar el Query al Worker
    /// [id_query]
    REQUEST_DESALOJO,
    REQUEST_DESALOJO_AGING,
    REQUEST_KILL,
    REQUEST_WRITE,
    REQUEST_READ,
    REQUEST_READ_DEL_WORKER,
	REQUEST_INFO,
	REQUEST_ACTION,
	REQUEST_KNOW
}request;

// CPU //

typedef enum{
    /// @brief [id_query, archivo_query, pc]
    EJECUTAR_QUERY=REQUEST_KNOW+1
}op_code_master;


typedef enum{    
    ///Formato: CREATE <NOMBRE_FILE>:<TAG>
    //La instrucción CREATE solicitará al módulo Storage la creación de un nuevo File con el Tag recibido por parámetro y con tamaño 0.
    CREATE=EJECUTAR_QUERY+1,
    //Formato: TRUNCATE <NOMBRE_FILE>:<TAG> <TAMAÑO>
    //La instrucción TRUNCATE solicitará al módulo Storage la modificación del tamaño del File y Tag indicados, asignando el tamaño recibido por parámetro (deberá ser múltiplo del tamaño de bloque).
    TRUNCATE,
    ///Formato: WRITE <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <CONTENIDO>
    ///La instrucción WRITE escribirá en la Memoria Interna los bytes correspondientes a partir de la dirección base del File:Tag. En caso de que la Memoria Interna no cuente con todas las páginas necesarias para satisfacer la operación, deberá solicitar el contenido faltante al módulo Storage.
    WRITE,
    ///Formato: READ <NOMBRE_FILE>:<TAG> <DIRECCIÓN BASE> <TAMAÑO>
    ///La instrucción READ leerá de la Memoria Interna los bytes correspondientes a partir de la dirección base del File y Tag pasados por parámetro, y deberá enviar dicha información al módulo Master.
    ///En caso de que la Memoria Interna no cuente con todas las páginas necesarias para satisfacer la operación, deberá solicitar el contenido faltante al módulo Storage.
    READ,
    ///Formato: TAG <NOMBRE_FILE_ORIGEN>:<TAG_ORIGEN> <NOMBRE_FILE_DESTINO>:<TAG_DESTINO>
    ///La instrucción TAG solicitará al módulo Storage la creación un nuevo File:Tag a partir del File y Tag origen pasados por parámetro.
    TAG,
    ///Formato: COMMIT <NOMBRE_FILE>:<TAG>
    ///La instrucción COMMIT, le indicará al Storage que no se realizarán más cambios sobre el File y Tag pasados por parámetro. 
    COMMIT,
    ///Formato: FLUSH <NOMBRE_FILE>:<TAG>
    ///Persistirá todas las modificaciones realizadas en Memoria Interna de un File:Tag en el Storage.
    ///Nota: Esta instrucción también deberá ser ejecutada implícitamente bajo las siguientes situaciones:
    ///Previo a la ejecución de un COMMIT.
    ///Previo a realizar el desalojo de la Query del Worker (para todos los File:Tag eventualmente modificados)
    FLUSH,
    ///Formato: DELETE <NOMBRE_FILE>:<TAG>
    ///La instrucción DELETE solicitará al módulo Storage la eliminación del File:Tag correspondiente.
    DELETE,
    INVALID_INSTRUCTION,
    NOOP,    
    ///Formato: END
    ///Esta instrucción da por finalizada la Query y le informa al módulo Master el fin de la misma.
    END
}instr_code;

typedef enum{
    /// @brief Least Recently Used
    R_LRU=END+1,
    /// @brief Least Recently Used
    R_CLOCK_M
}replace_algorithm;

typedef enum{
    WORK_IN_PROGRESS=R_CLOCK_M+1,
    COMMITED
}state_metadata;

// CPU //

typedef enum{
    /// @brief Esta operación creará un nuevo File dentro del FS. Para ello recibirá el nombre del File y un Tag inicial para crearlo.
    ///Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.
    CREATE_FILE = COMMITED+1,
    /// @brief Esta operación se encargará de modificar el tamaño del File:Tag especificados agrandando o achicando el tamaño del mismo para reflejar el nuevo tamaño deseado (actualizando la metadata necesaria).
    /// Al incrementar el tamaño del File, se le asignarán tantos bloques lógicos (hard links) como sea necesario. Inicialmente, todos ellos deberán apuntar el bloque físico nro 0.
    /// Al reducir el tamaño del File, se deberán desasignar tantos bloques lógicos como sea necesario (empezando por el final del archivo). Si el bloque físico al que apunta el bloque lógico eliminado no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.
    TRUNCATE_FILE,
    /// @brief Esta operación creará una copia completa del directorio nativo correspondiente al Tag de origen en un nuevo directorio correspondiente al Tag destino y modificará en el archivo de metadata del Tag destino para que el mismo se encuentre en estado WORK_IN_PROGRESS.
    TAG_FILE,
    /// @brief Confirmará un File:Tag pasado por parámetro. En caso de que un Tag ya se encuentre confirmado, esta operación no realizará nada. Para esto se deberá actualizar el archivo metadata del Tag pasando su estado a “COMMITED”.
    /// Se deberá, por cada bloque lógico, buscar si existe algún bloque físico que tenga el mismo contenido (utilizando el hash y archivo blocks_hash_index.config). En caso de encontrar uno, se deberá liberar el bloque físico actual y reapuntar el bloque lógico al bloque físico pre-existente. En caso contrario, simplemente se agregará el hash del nuevo contenido al archivo blocks_hash_index.config.
    COMMIT_TAG,
    /// @brief Esta operación recibirá el contenido de un bloque lógico de un File:Tag y guardará los cambios en el bloque físico correspondiente, siempre y cuando el File:Tag no se encuentre en estado COMMITED y el bloque lógico se encuentre asignado.
    /// Si el bloque lógico a escribir fuera el único referenciando a su bloque físico asignado, se escribirá dicho bloque físico directamente. En caso contrario, se deberá buscar un nuevo bloque físico, escribir en el mismo y asignarlo al bloque lógico en cuestión.
    WRITE_BLOCK,
    /// @brief Dado un File:Tag y número de bloque lógico, la operación de lectura obtendrá y devolverá el contenido del mismo.
    READ_BLOCK,
    /// @brief Esta operación eliminará el directorio correspondiente al File:Tag indicado. Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.
    DELETE_TAG
    
}storage_operation;

typedef enum{
    WRITE_BLOCK_NOT_ERROR = DELETE_TAG +1,
}storage_operation_additional;
typedef enum{
    //Una operación quiere realizar una acción sobre un File:Tag que no existe (salvo la operación de CREATE que crea un nuevo File:Tag).
    FILE_NOT_FOUND = WRITE_BLOCK_NOT_ERROR+1,
    /// @brief Una operación quiere realizar una acción sobre un tag que no existe, salvo la operación de TAG que crea un nuevo Tag.
    TAG_NOT_FOUND,
    /// @brief Al intentar asignar un nuevo bloque físico, no se encuentra ninguno disponible.
    INSUFFICIENT_SPACE,
    /// @brief Una query intenta escribir o truncar un File:Tag que se encuentre en estado COMMITED.
    WRITE_NO_PERMISSION,
    /// @brief Un bobo intento crear un tag ya existente, no se que le pasa.
    TAG_YA_EXISTENTE_SACA_LA_MANO_DE_AHI,
    /// @brief Una query intenta leer o escribir por fuera del tamaño del File:Tag.
    READ_WRITE_OVERFLOW
}errors_operation;

typedef enum{
    QUERY_END=READ_WRITE_OVERFLOW+1
}master_operation;

typedef enum{
    BLOCK_SIZE = QUERY_END+1,
    GET_BLOCK_DATA,
    INSTRUCTION_ERROR,
    GET_DATA,
    RETURN_BLOCK_DATA,
}storage_worker_data;

//Como te complicas la vida pa, ni hace falta esto, simplemente respondes con la instrucción y se sabe que fue OK... 
typedef enum{
    /// @brief Esta operación creará un nuevo File dentro del FS. Para ello recibirá el nombre del File y un Tag inicial para crearlo.
    ///Deberá crear el archivo de metadata en estado WORK_IN_PROGRESS y no asignarle ningún bloque.
    OK_CREATE_FILE = RETURN_BLOCK_DATA+1,
    /// @brief Esta operación se encargará de modificar el tamaño del File:Tag especificados agrandando o achicando el tamaño del mismo para reflejar el nuevo tamaño deseado (actualizando la metadata necesaria).
    /// Al incrementar el tamaño del File, se le asignarán tantos bloques lógicos (hard links) como sea necesario. Inicialmente, todos ellos deberán apuntar el bloque físico nro 0.
    /// Al reducir el tamaño del File, se deberán desasignar tantos bloques lógicos como sea necesario (empezando por el final del archivo). Si el bloque físico al que apunta el bloque lógico eliminado no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.
    OK_TRUNCATE_FILE,
    /// @brief Esta operación creará una copia completa del directorio nativo correspondiente al Tag de origen en un nuevo directorio correspondiente al Tag destino y modificará en el archivo de metadata del Tag destino para que el mismo se encuentre en estado WORK_IN_PROGRESS.
    OK_TAG_FILE,
    /// @brief Confirmará un File:Tag pasado por parámetro. En caso de que un Tag ya se encuentre confirmado, esta operación no realizará nada. Para esto se deberá actualizar el archivo metadata del Tag pasando su estado a “COMMITED”.
    /// Se deberá, por cada bloque lógico, buscar si existe algún bloque físico que tenga el mismo contenido (utilizando el hash y archivo blocks_hash_index.config). En caso de encontrar uno, se deberá liberar el bloque físico actual y reapuntar el bloque lógico al bloque físico pre-existente. En caso contrario, simplemente se agregará el hash del nuevo contenido al archivo blocks_hash_index.config.
    OK_COMMIT_TAG,
    /// @brief Esta operación recibirá el contenido de un bloque lógico de un File:Tag y guardará los cambios en el bloque físico correspondiente, siempre y cuando el File:Tag no se encuentre en estado COMMITED y el bloque lógico se encuentre asignado.
    /// Si el bloque lógico a escribir fuera el único referenciando a su bloque físico asignado, se escribirá dicho bloque físico directamente. En caso contrario, se deberá buscar un nuevo bloque físico, escribir en el mismo y asignarlo al bloque lógico en cuestión.
    OK_WRITE_BLOCK,
    /// @brief Dado un File:Tag y número de bloque lógico, la operación de lectura obtendrá y devolverá el contenido del mismo.
    OK_READ_BLOCK,
    /// @brief Esta operación eliminará el directorio correspondiente al File:Tag indicado. Al realizar esta operación, si el bloque físico al que apunta cada bloque lógico eliminado no es referenciado por ningún otro File:Tag, deberá ser marcado como libre en el bitmap.
    OK_DELETE_TAG
}ok_storage_operation;

typedef enum{
    QUERY_DESALOJADA = OK_DELETE_TAG+1
}master_worker_data;

#endif