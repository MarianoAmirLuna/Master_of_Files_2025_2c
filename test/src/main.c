#include "main.h"
#include "../../storage/src/funciones_generales.h"

int main(int argc, char* argv[]) {
    
    create_log("test", LOG_LEVEL_TRACE);
    log_violet(logger, "%s", "Hola soy Test");
    crear_directorio("pepito", "/home/utnso");
    return 0;
}