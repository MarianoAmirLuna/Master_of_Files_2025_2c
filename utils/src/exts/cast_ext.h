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
    log_error(logger, "No se encontró instrucción %s:%d", __func__,__LINE__);
    return INVALID_INSTRUCTION;
}

int convert_instr_code_to_storage_operation(instr_code code){
    if(code == CREATE)
    {
        return CREATE_FILE;
    }
    if(code == TRUNCATE)
    {
        return TRUNCATE_FILE;
    }
    if(code == TAG)
    {
        return TAG_FILE;
    }
    if(code == COMMIT)
    {
        return COMMIT_TAG;
    }
    if(code == WRITE)
    {
        return WRITE_BLOCK;
    }
    if(code == READ)
    {
        return READ_BLOCK;
    }
    if(code == DELETE)
    {
        return DELETE_TAG;
    }
    return code;
}

storage_operation cast_storage_oper(char* code){
    if(string_equals_ignore_case(code, "CREATE_FILE")){
        return CREATE_FILE;
    }
    if(string_equals_ignore_case(code, "TRUNCATE_FILE")){
        return TRUNCATE_FILE;
    }
    if(string_equals_ignore_case(code, "TAG_FILE")){
        return TAG_FILE;
    }
    if(string_equals_ignore_case(code, "COMMIT_TAG")){
        return COMMIT_TAG;
    }
    if(string_equals_ignore_case(code, "WRITE_BLOCK")){
        return WRITE_BLOCK;
    }
    if(string_equals_ignore_case(code, "READ_BLOCK")){
        return READ_BLOCK;
    }
    if(string_equals_ignore_case(code, "DELETE_TAG")){
        return DELETE_TAG;
    }
    return ERROR;
}

errors_operation cast_error_operation(char* code){
    if(string_equals_ignore_case(code, "FILE_NOT_FOUND")){
        return FILE_NOT_FOUND;
    }
    if(string_equals_ignore_case(code, "TAG_NOT_FOUND")){
        return TAG_NOT_FOUND;
    }
    if(string_equals_ignore_case(code, "INSUFFICIENT_SPACE")){
        return INSUFFICIENT_SPACE;
    }
    if(string_equals_ignore_case(code, "WRITE_NO_PERMISSION")){
        return WRITE_NO_PERMISSION;
    }
    if(string_equals_ignore_case(code, "READ_WRITE_OVERFLOW")){
        return READ_WRITE_OVERFLOW;
    }

    return ERROR;
}
    
#endif