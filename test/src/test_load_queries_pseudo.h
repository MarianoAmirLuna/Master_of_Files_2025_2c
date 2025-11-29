#include "commons/string.h"
#include "utils/enums.h"
#include "stdio.h"
#include "string.h"
#include "libs/logger.h"
#include "exts/log_ext.h"
#include "exts/list_ext.h"
#include "exts/string_ext.h"


void parse_code(instr_code code, char* instr){
    if(code == CREATE){
        char* file = NULL;
        char* tag = NULL;
        char** spl= string_split(file_y_tag, ":");
        file = malloc(strlen(spl[0])+1);
        tag = malloc(strlen(spl[1])+1);
        strcpy(file, spl[0]);
        strcpy(tag, spl[1]);
        
        /*char* file=string_new();
        char* tag= string_new();
        get_tag_file(instr, file,tag);*/

        /*char** spl = string_split(instr, ":");
        char* file = string_new();
        char* tag = string_new();
        strcpy(file, spl[0]);
        strcpy(tag, spl[1]);*/
        log_orange(logger, "File: %s, Tag: %s", file, tag);
        //string_array_destroy(spl);
        free(file);
        free(tag);
    }
    if(code == READ){
        char* left = string_new();
        char* middle = string_new();
        char* right = string_new();
        get_two_space_instr(instr, left, middle, right);
        log_orange(logger, "LEFT: %s", left);
        log_orange(logger, "MIDDLE: %s", middle);
        log_orange(logger, "RIGHT: %s", right);


        /*int a=0;
        int b=0;
        get_both_num_instr(right, &a, &b);*/
        log_orange(logger, "A= %d, B=%d", atoi(middle), atoi(right));
        return;
    }
    if(code == TAG){
        char* left_side = string_new();
        char* right_side = string_new();
        get_space_instr(instr, left_side, right_side);

        char* file_a = string_new();
        char* tag_a = string_new();
        char* file_b = string_new();
        char* tag_b = string_new();
        get_tag_file(left_side, file_a, tag_a);
        get_tag_file(right_side, file_b, tag_b);

        log_pink(logger,"Con TAG De lado izquierdo File: %s, Tag: %s, de lado derecho File: %s, Tag: %s", file_a, tag_a, file_b, tag_b);
        
    }
    if(code == TRUNCATE){
        char* left_side = string_new();
        char* right_side = string_new();
        get_space_instr(instr, left_side, right_side);

        char* file = string_new();
        char* tag = string_new();
        get_tag_file(left_side, file, tag);
        int tam = atoi(right_side);

        log_pink(logger,"Con TRUNCATE File: %s, Tag: %s, Tam: %d", file, tag, tam);
    }
}