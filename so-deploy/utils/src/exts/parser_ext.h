#ifndef EXTS_PARSER_EXT
#define EXTS_PARSER_EXT

#ifndef UTILS_ENUMS_H
#include "utils/enums.h"
#endif

#include "commons/string.h"

#ifndef CAST_EXT_H
#include "exts/cast_ext.h"
#endif

/*
CREATE MATERIAS:BASE 
TRUNCATE MATERIAS:BASE 1024
WRITE MATERIAS:BASE 0 SISTEMAS_OPERATIVOS
FLUSH MATERIAS:BASE
COMMIT MATERIAS:BASE
READ MATERIAS:BASE 0 8
TAG MATERIAS:BASE MATERIAS:V2
DELETE MATERIAS:BASE
WRITE MATERIAS:V2 0 SISTEMAS_OPERATIVOS_2
COMMIT MATERIAS:V2
END
*/


void parser_instrs(char* line){
    char** spl = string_split(line, " ");
    int sz = string_array_size(spl);
    
    char* charcode= spl[0];
    instr_code code = cast_code(charcode);

    if(sz == 0){
        
        //Sera solo END no puede ser otra cosa
    }

    //string_
}

#endif 