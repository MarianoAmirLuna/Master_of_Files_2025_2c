1. Se envia doble respuesta al Worker
- Siempre se envia un SUCCESS, incluso cuando antes se envio un error.

2. Errores no manejados
- Hay errores en los cuales no se envia a Worker y solo se loguea. 
Los detectados por ahora son:
  - READ_BLOCK: Bloque logico no existe o fopen falla.
  - TAG_FILE: Tag destino ya existe.
  - CREATE_FILE: Parametros NULL.
  - WRITE_BLOCK: Metadata NULL.

3. Validaciones faltantes
- Insuficiente espacio en WRITE_BLOCK.
- Overflow en READ_BLOCK y WRITE_BLOCK (Validar que (bloque_solicitado < total_bloques_archivo)).

4. Otros errores y faltantes en el codigo (Paja de escribirlos todos)

