1. Se envia doble respuesta al Worker
- Siempre se envia un SUCCESS, incluso cuando antes se envio un error.

PD: Aparentemente no afecta porque Worker no hace nada cuando se recibe SUCCESS.
-- es una pelotudez


2. Errores no manejados
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



4. Otros errores y faltantes en el codigo (Paja de escribirlos todos)


5. Que pasa cuando se realiza un flush desde worker y uno de los archivos que se intenta modificar esta ya comiteado? Puede derivar en un bucle aparentemente, porque Master indica desalojar y en dicho caso Worker genera un flush, lo cual deriva en un error de permisos, y por lo tanto el mismo error.
/* 
  ver con dimi que hacer
*/
