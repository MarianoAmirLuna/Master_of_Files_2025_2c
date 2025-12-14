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

int cast_state_metadata(char* state){
    if(string_equals_ignore_case(state, "WORK_IN_PROGRESS")){
        return WORK_IN_PROGRESS;
    }
    if(string_equals_ignore_case(state, "COMMITED")){
        return COMMITED;
    }
    log_error(logger, "No se encontró estado %s:%d", __func__,__LINE__);
    return -1;
}

char* get_opcode_as_string(int opcode){
    if(opcode == SUCCESS){
        return "SUCCESS";
    }
    if(opcode == ENDOFLINE){
        return "ENDOFLINE";
    }
    if(opcode == END_OF_IO){
        return "END_OF_IO";
    }
    if(opcode == INTERRUPT){
        return "INTERRUPT";
    }
    if(opcode == SEGMENTATION_FAULT){
        return "SEGMENTATION_FAULT";
    }
    if(opcode == PROCESS_CANT_INITIALIZED){
        return "PROCESS_CANT_INITIALIZED";
    }
    if(opcode == ACTUAL_STATUS){
        return "ACTUAL_STATUS";
    }
    if(opcode == NOTEXISTS){
        return "NOTEXISTS";
    }
    if(opcode == RESCHEDULE){
        return "RESCHEDULE";
    }
    if(opcode == QUERY_ID){
        return "QUERY_ID";
    }
    if(opcode == PROCESS_FINISHED){
        return "PROCESS_FINISHED";
    }
    if(opcode == ERROR){
        return "ERROR";
    }
    if(opcode == REQUEST_EXECUTE_QUERY){
        return "REQUEST_EXECUTE_QUERY";
    }
    if(opcode == REQUEST_END_QUERY){
        return "REQUEST_END_QUERY";
    }
    if(opcode == REQUEST_CONTEXT_EXECUTION){
        return "REQUEST_CONTEXT_EXECUTION";
    }
    if(opcode == REQUEST_LIST_INSTRUCTIONS){
        return "REQUEST_LIST_INSTRUCTIONS";
    }
    if(opcode == REQUEST_CHECK_SPACE_MEMORY){
        return "REQUEST_CHECK_SPACE_MEMORY";
    }
	if(opcode == REQUEST_INSTRUCTIONS_MEMORY){
        return "REQUEST_INSTRUCTIONS_MEMORY";
    }
    if(opcode == TUVE_UNA_RESPUESTA_DEL_PUTO_STORAGE)
    {
        return "TUVE_UNA_RESPUESTA_DEL_PUTO_STORAGE";
    }
    if(opcode == REQUEST_DESALOJO){
        return "REQUEST_DESALOJO";
    }
    if(opcode == REQUEST_KILL){
        return "REQUEST_KILL";
    }
    if(opcode == REQUEST_WRITE){
        return "REQUEST_WRITE";
    }
    if(opcode == REQUEST_READ){
        return "REQUEST_READ";
    }
	if(opcode == REQUEST_INFO){
        return "REQUEST_INFO";
    }
	if(opcode == REQUEST_ACTION){
        return "REQUEST_ACTION";
    }
	if(opcode == REQUEST_KNOW){
        return "REQUEST_KNOW";
    }
    if(opcode == EJECUTAR_QUERY){
        return "EJECUTAR_QUERY";
    }
    if(opcode == CREATE_FILE){
        return "CREATE_FILE";
    }
    if(opcode == TRUNCATE_FILE){
        return "TRUNCATE_FILE";
    }
    if(opcode == TAG_FILE){
        return "TAG_FILE";
    }
    if(opcode == COMMIT_TAG){
        return "COMMIT_TAG";
    }
    if(opcode == WRITE_BLOCK){
        return "WRITE_BLOCK";
    }
    if(opcode == READ_BLOCK){
        return "READ_BLOCK";
    }
    if(opcode == DELETE_TAG){
        return "DELETE_TAG";
    }
    if(opcode == WRITE_BLOCK_NOT_ERROR){
        return "WRITE_BLOCK_NOT_ERROR";
    }
    if(opcode == TAG_NOT_FOUND){
        return "TAG_NOT_FOUND";
    }
    if(opcode == INSUFFICIENT_SPACE){
        return "INSUFFICIENT_SPACE";
    }
    if(opcode == WRITE_NO_PERMISSION){
        return "WRITE_NO_PERMISSION";
    }
    if(opcode == READ_WRITE_OVERFLOW){
        return "READ_WRITE_OVERFLOW";
    }
    if(opcode == QUERY_END){
        return "QUERY_END";
    }
    if(opcode == BLOCK_SIZE){
        return "BLOCK_SIZE";
    }
    if(opcode == GET_BLOCK_DATA){
        return "GET_BLOCK_DATA";
    }
    if(opcode == INSTRUCTION_ERROR){
        return "INSTRUCTION_ERROR";
    }
    if(opcode == RETURN_BLOCK_DATA){
        return "RETURN_BLOCK_DATA";
    }
    if(opcode == QUERY_DESALOJADA){
        return "QUERY_DESALOJADA";
    }
    if(opcode == BLOCK_SIZE)
    {
        return "BLOCK_SIZE";
    }
    if(opcode == GET_BLOCK_DATA)
    {
        return "GET_BLOCK_DATA";
    }
    if(opcode == INSTRUCTION_ERROR)
    {
        return "INSTRUCTION_ERROR";
    }
    if(opcode == GET_DATA)
    {
        return "GET_DATA";
    }
    if(opcode == RETURN_BLOCK_DATA)
    {
        return "RETURN_BLOCK_DATA";
    }
    return NULL;
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
    if(code == END)
    {
        return QUERY_END;
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