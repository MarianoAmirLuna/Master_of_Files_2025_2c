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
    /// @brief No existe
    NOTEXISTS,
    /// @brief Replanificación
    RESCHEDULE,
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
    REQUEST_WRITE,
    REQUEST_READ,
	REQUEST_INFO,
	REQUEST_ACTION,
	REQUEST_KNOW
}request;

typedef enum{    
    ///Formato: CREATE <NOMBRE_FILE>:<TAG>
    //La instrucción CREATE solicitará al módulo Storage la creación de un nuevo File con el Tag recibido por parámetro y con tamaño 0.
    CREATE=REQUEST_KNOW+1,
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
    ///Formato: END
    ///Esta instrucción da por finalizada la Query y le informa al módulo Master el fin de la misma.
    END
}instr_code;

typedef enum{
    /// @brief Least Recently Used
    R_LRU,
    /// @brief Least Recently Used
    R_CLOCK_M
}replace_algorithm;

#endif