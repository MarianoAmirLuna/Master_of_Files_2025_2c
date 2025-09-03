#ifndef CAST_EXT_H
#define CAST_EXT_H
#include "string.h"
#include "commons/string.h"
#ifndef UTILS_ENUMS_H
#include "utils/enums.h"
#endif
/*
instr_code cast_code(char* name){
    if(string_contains(name, "\n"))
        name[strcspn(name, "\n")] = 0;
    if(string_equals_ignore_case(name, "DUMP_MEMORY"))
        return DUMP_MEMORY;
    if(string_equals_ignore_case(name, "INIT_PROC"))
        return INIT_PROC;
    if(string_equals_ignore_case(name, "DUMP_MEMORY"))
        return DUMP_MEMORY;
    if(string_equals_ignore_case(name, "IO"))
        return IO;
    if(string_equals_ignore_case(name, "EXIT"))
        return INSTR_SYSCALL_EXIT;
    if(string_equals_ignore_case(name, "NOOP"))
        return NOOP;
    if(string_equals_ignore_case(name, "WRITE"))
        return WRITE;
    if(string_equals_ignore_case(name, "READ"))
        return READ;
    if(string_equals_ignore_case(name, "GOTO"))
        return GOTO;
    log_error(logger, "Error valor inválido exit(1) invoked");
    exit(1);
}

replace_tlb cast_replace_tlb(char* n)
{
    if(string_equals_ignore_case(n, "TLB"))
        return R_TLB;
    if(string_equals_ignore_case(n, "FIFO"))
        return R_FIFO;
    if(string_equals_ignore_case(n, "LRU"))
        return R_LRU;
    log_error(logger, "Error valor inválido exit(1) invoked");
    exit(1);
}

replace_cache cast_replace_cache(char* n){
    if(string_equals_ignore_case(n, "CLOCK"))
        return CLOCK;
    if(string_equals_ignore_case(n, "CLOCK-M"))
        return CLOCK_M;
    log_error(logger, "Error valor inválido exit(1) invoked");
    exit(1);
}*/

#endif