#ifndef CAST_EXT_H
#define CAST_EXT_H
#include "string.h"
#include "commons/string.h"
#ifndef UTILS_ENUMS_H
#include "utils/enums.h"
#endif

char* get_string_state(state_metadata state){
    if(state == WORK_IN_PROGRESS){
        return "WORK_IN_PROGRESS";
    }
    if(state == COMMITED){
        return "COMMITED";
    }
    return string_new();
}

instr_code cast_code(char* code){
    if(string_equals_ignore_case(code,"CREATE")){
        return CREATE;
    }
    if(string_equals_ignore_case(code,"TRUNCATE")){
        return TRUNCATE;
    }
    if(string_equals_ignore_case(code,"WRITE")){
        return WRITE;
    }
    if(string_equals_ignore_case(code,"READ")){
        return READ;
    }
    if(string_equals_ignore_case(code,"TAG")){
        return TAG;
    }
    if(string_equals_ignore_case(code,"COMMIT")){
        return COMMIT;
    }
    if(string_equals_ignore_case(code,"FLUSH")){
        return FLUSH;
    }
    if(string_equals_ignore_case(code,"DELETE")){
        return DELETE;
    }
    if(string_equals_ignore_case(code,"END")){
        return END;
    }
}
#endif