#include "main.h"
#include "../../storage/src/funciones_generales.h"
#include "../../storage/src/inicializacion_storage.h"
#include "../../storage/src/comunicacion_worker.h"
#include "../../storage/src/test.h"

int main(int argc, char* argv[]) {
    
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    test_create_file();
    //crear_directorio("pepito", "/home/utnso");

    return 0;
}