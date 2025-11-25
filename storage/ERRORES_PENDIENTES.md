1. Se envia doble respuesta al Worker
- Siempre se envia un SUCCESS, incluso cuando antes se envio un error.

PD: Aparentemente no afecta porque Worker no hace nada cuando se recibe SUCCESS.
-- es una pelotudez


Hola, el que se puso a leer el codigo para pegarse un susto, GGG :D

2. Errores no maneCCjados
- Hay errores en los cuales no se envia a Worker y solo se loguea. 
Los detectados por ahora son:
  - READ_BLOCK: Bloque logico no existe o fopen falla. // controlado el fuera de limite
  - TAG_FILE: Tag destino ya existe. // No requiere enviarse, se opera sobre el tag indicado en
      el futuro directamente.
  - CREATE_FILE: Parametros NULL. // Ya se controla
  - WRITE_BLOCK: Metadata NULL. // re factorizado

3. Validaciones faltantes
- Insuficiente espacio en WRITE_BLOCK. // refactorizada
- Overflow en READ_BLOCK y WRITE_BLOCK (Validar que (bloque_solicitado < total_bloques_archivo)). // ¿lo acabamos de agregar?

5. Que pasa cuando se realiza un flush desde worker y uno de los archivos que se intenta modificar esta ya comiteado? Puede derivar en un bucle aparentemente, porque Master indica desalojar y en dicho caso Worker genera un flush, lo cual deriva en un error de permisos, y por lo tanto el mismo error.
/* 
  ver con dimi que hacer
*/



---

6. TAG_FILE - Si falla la copia, eliminar el nuevo directorio creado.
// no deberia fallar en teoria al ser un read, no creo sea necesario pero lo tenemos en cuenta





